// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "axivionquery.h"

#include "axivionplugin.h"
#include "axivionsettings.h"

#include <utils/processenums.h>
#include <utils/qtcassert.h>

#include <QUrl>

using namespace Utils;

namespace Axivion::Internal {

AxivionQuery::AxivionQuery(QueryType type, const QStringList &parameters)
    : m_type(type)
    , m_parameters(parameters)
{
}

QString AxivionQuery::toString() const
{
    QString query = "/api"; // common for all
    switch (m_type) {
    case NoQuery:
        return {};
    case DashboardInfo:
        return query;
    case ProjectInfo:
        QTC_ASSERT(m_parameters.size() == 1, return {});
        query += "/projects/" + QUrl::toPercentEncoding(m_parameters.first());
        return query;
    }

    return {};
}

AxivionQueryRunner::AxivionQueryRunner(const AxivionQuery &query, QObject *parent)
    : QObject(parent)
{
    const AxivionSettings *settings = AxivionPlugin::settings();
    const AxivionServer server = settings->server;

    QStringList args = server.curlArguments();
    args << "-i";
    args << "--header" << "Authorization: AxToken " + server.token;

    QString url = server.dashboard;
    while (url.endsWith('/')) url.chop(1);
    url += query.toString();
    args << url;

    m_process.setCommand({settings->curl, args});
    connect(&m_process, &QtcProcess::done, this, [this]{
        if (m_process.result() != ProcessResult::FinishedWithSuccess) {
            const int exitCode = m_process.exitCode();
            if (m_process.exitStatus() == QProcess::NormalExit
                    && (exitCode == 35 || exitCode == 60)
                    && AxivionPlugin::handleCertificateIssue()) {
                // prepend -k for re-requesting same query
                CommandLine cmdline = m_process.commandLine();
                cmdline.prependArgs({"-k"});
                m_process.close();
                m_process.setCommand(cmdline);
                start();
                return;
            }
            emit resultRetrieved(m_process.readAllStandardError());
        } else {
            emit resultRetrieved(m_process.readAllStandardOutput());
        }
        emit finished();
    });
}

void AxivionQueryRunner::start()
{
    QTC_ASSERT(!m_process.isRunning(), return);
    m_process.start();
}

} // Axivion::Internal
