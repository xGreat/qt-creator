/*
 * Copyright (C) 2022-current by Axivion GmbH
 * https://www.axivion.com/
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
 */

#include "dashboardclient.h"

#include "axivionsettings.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QLatin1String>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPromise>

#include <memory>

namespace Axivion::Internal
{

Credential::Credential(const QString &apiToken)
    : m_authorizationValue(QByteArrayLiteral(u8"AxToken ") + apiToken.toUtf8())
{
}

const QByteArray &Credential::authorizationValue() const
{
    return m_authorizationValue;
}

QFuture<Credential> CredentialProvider::getCredential()
{
    return QtFuture::makeReadyFuture(Credential(settings().server.token));
}

ClientData::ClientData(Utils::NetworkAccessManager &networkAccessManager)
    : networkAccessManager(networkAccessManager),
      credentialProvider(std::make_unique<CredentialProvider>())
{
}

DashboardClient::DashboardClient(Utils::NetworkAccessManager &networkAccessManager)
    : m_clientData(std::make_shared<ClientData>(networkAccessManager))
{
}

using ResponseData = Utils::expected<DataWithOrigin<QByteArray>, Error>;

static constexpr int httpStatusCodeOk = 200;
static const QLatin1String jsonContentType{ "application/json" };

static ResponseData readResponse(QNetworkReply &reply, QAnyStringView expectedContentType)
{
    QNetworkReply::NetworkError error = reply.error();
    int statusCode = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString contentType = reply.header(QNetworkRequest::ContentTypeHeader)
                              .toString()
                              .split(';')
                              .constFirst()
                              .trimmed()
                              .toLower();
    if (error == QNetworkReply::NetworkError::NoError
            && statusCode == httpStatusCodeOk
            && contentType == expectedContentType) {
        return DataWithOrigin(reply.url(), reply.readAll());
    }
    if (contentType == jsonContentType) {
        try {
            return tl::make_unexpected(DashboardError(
                reply.url(),
                statusCode,
                reply.attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(),
                Dto::ErrorDto::deserialize(reply.readAll())));
        } catch (const Dto::invalid_dto_exception &) {
            // ignore
        }
    }
    if (statusCode != 0) {
        return tl::make_unexpected(HttpError(
            reply.url(),
            statusCode,
            reply.attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(),
            QString::fromUtf8(reply.readAll()))); // encoding?
    }
    return tl::make_unexpected(
        NetworkError(reply.url(), error, reply.errorString()));
}

template<typename T>
static Utils::expected<DataWithOrigin<T>, Error> parseResponse(ResponseData rawBody)
{
    if (!rawBody)
        return tl::make_unexpected(std::move(rawBody.error()));
    try {
        T data = T::deserialize(rawBody.value().data);
        return DataWithOrigin(std::move(rawBody.value().origin),
                              std::move(data));
    } catch (const Dto::invalid_dto_exception &e) {
        return tl::make_unexpected(GeneralError(std::move(rawBody.value().origin),
                                                QString::fromUtf8(e.what())));
    }
}

static void fetch(QPromise<ResponseData> promise,
                  std::shared_ptr<ClientData> clientData,
                  const QUrl &url,
                  const Credential &credential)
{
    QNetworkRequest request{ url };
    request.setRawHeader(QByteArrayLiteral(u8"Accept"),
                         QByteArray(jsonContentType.data(), jsonContentType.size()));
    request.setRawHeader(QByteArrayLiteral(u8"Authorization"),
                         credential.authorizationValue());
    QByteArray ua = QByteArrayLiteral(u8"Axivion")
                    + QCoreApplication::applicationName().toUtf8()
                    + QByteArrayLiteral(u8"Plugin/")
                    + QCoreApplication::applicationVersion().toUtf8();
    request.setRawHeader(QByteArrayLiteral(u8"X-Axivion-User-Agent"), ua);
    QNetworkReply *reply = clientData->networkAccessManager.get(request);
    QObject::connect(reply,
                     &QNetworkReply::finished,
                     reply,
                     [promise = std::move(promise), reply]() mutable {
                         promise.addResult(readResponse(*reply, jsonContentType));
                         promise.finish();
                         reply->deleteLater();
                     });
}

static QFuture<ResponseData> fetch(std::shared_ptr<ClientData> clientData,
                                   const std::optional<QUrl> &base,
                                   const QUrl &target)
{
    QPromise<ResponseData> promise;
    promise.start();
    QFuture<ResponseData> future = promise.future();
    QUrl url = base ? base->resolved(target) : target;
    QFutureWatcher<Credential> *watcher = new QFutureWatcher<Credential>(&clientData->networkAccessManager);
    QObject::connect(watcher,
                     &QFutureWatcher<Credential>::finished,
                     &clientData->networkAccessManager,
                     [promise = std::move(promise), clientData, url = std::move(url), watcher]() mutable {
                         fetch(std::move(promise),
                               std::move(clientData),
                               url,
                               watcher->result());
                         watcher->deleteLater();;
                     });
    watcher->setFuture(clientData->credentialProvider->getCredential());
    return future;
}

QFuture<DashboardClient::RawProjectInfo> DashboardClient::fetchProjectInfo(const QString &projectName)
{
    const AxivionServer &server = settings().server;
    QString dashboard = server.dashboard;
    if (!dashboard.endsWith(QLatin1Char('/')))
        dashboard += QLatin1Char('/');
    QUrl url = QUrl(dashboard)
        .resolved(QUrl(QStringLiteral(u"api/projects/")))
        .resolved(QUrl(projectName));
    return fetch(this->m_clientData, std::nullopt, url)
        .then(QtFuture::Launch::Async, &parseResponse<Dto::ProjectInfoDto>);
}

} // namespace Axivion::Internal
