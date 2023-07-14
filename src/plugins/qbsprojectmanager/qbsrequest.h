// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <projectexplorer/buildstep.h>

#include <solutions/tasking/tasktree.h>

#include <QJsonObject>

namespace QbsProjectManager::Internal {

class QbsSession;

class QbsRequest final : public QObject
{
    Q_OBJECT

public:
    ~QbsRequest() override;

    void setSession(QbsSession *session);
    void setRequestData(const QJsonObject &requestData) { m_requestData = requestData; }
    void start();

signals:
    void done(bool success);
    void progressChanged(int progress, const QString &info); // progress in %
    void outputAdded(const QString &output, ProjectExplorer::BuildStep::OutputFormat format);
    void taskAdded(const ProjectExplorer::Task &task);

private:
    QbsSession *m_session = nullptr;
    std::optional<QJsonObject> m_requestData;
    bool m_isRunning = false;
    QString m_description;
    int m_maxProgress = 100;
};

class QbsRequestTaskAdapter : public Tasking::TaskAdapter<QbsRequest>
{
public:
    QbsRequestTaskAdapter() { connect(task(), &QbsRequest::done, this, &TaskInterface::done); }

private:
    void start() final { task()->start(); }
};

} // namespace QbsProjectManager::Internal

TASKING_DECLARE_TASK(QbsRequestTask, QbsProjectManager::Internal::QbsRequestTaskAdapter);
