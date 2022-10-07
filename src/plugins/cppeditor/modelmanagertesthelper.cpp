// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "modelmanagertesthelper.h"

#include "cpptoolstestcase.h"
#include "cppworkingcopy.h"
#include "projectinfo.h"

#include <projectexplorer/session.h>

#include <QtTest>

#include <cassert>

namespace CppEditor::Tests {

TestProject::TestProject(const QString &name, QObject *parent, const Utils::FilePath &filePath) :
    ProjectExplorer::Project("x-binary/foo", filePath),
    m_name(name)
{
    setParent(parent);
    setId(Utils::Id::fromString(name));
    setDisplayName(name);
    qRegisterMetaType<QSet<QString> >();
}

ModelManagerTestHelper::ModelManagerTestHelper(QObject *parent,
                                               bool testOnlyForCleanedProjects)
    : QObject(parent)
    , m_testOnlyForCleanedProjects(testOnlyForCleanedProjects)

{
    CppModelManager *mm = CppModelManager::instance();
    connect(this, &ModelManagerTestHelper::aboutToRemoveProject,
            mm, &CppModelManager::onAboutToRemoveProject);
    connect(this, &ModelManagerTestHelper::projectAdded,
            mm, &CppModelManager::onProjectAdded);
    connect(mm, &CppModelManager::sourceFilesRefreshed,
            this, &ModelManagerTestHelper::sourceFilesRefreshed);
    connect(mm, &CppModelManager::gcFinished,
            this, &ModelManagerTestHelper::gcFinished);

    cleanup();
    QVERIFY(Internal::Tests::VerifyCleanCppModelManager::isClean(m_testOnlyForCleanedProjects));
}

ModelManagerTestHelper::~ModelManagerTestHelper()
{
    cleanup();
    QVERIFY(Internal::Tests::VerifyCleanCppModelManager::isClean(m_testOnlyForCleanedProjects));
}

void ModelManagerTestHelper::cleanup()
{
    CppModelManager *mm = CppModelManager::instance();
    QList<ProjectInfo::ConstPtr> pies = mm->projectInfos();
    for (Project * const p : std::as_const(m_projects)) {
        ProjectExplorer::SessionManager::removeProject(p);
        emit aboutToRemoveProject(p);
    }

    if (!pies.isEmpty())
        waitForFinishedGc();
}

ModelManagerTestHelper::Project *ModelManagerTestHelper::createProject(
        const QString &name, const Utils::FilePath &filePath)
{
    auto tp = new TestProject(name, this, filePath);
    m_projects.push_back(tp);
    ProjectExplorer::SessionManager::addProject(tp);
    emit projectAdded(tp);
    return tp;
}

QSet<QString> ModelManagerTestHelper::updateProjectInfo(
        const ProjectInfo::ConstPtr &projectInfo)
{
    resetRefreshedSourceFiles();
    CppModelManager::instance()->updateProjectInfo(projectInfo).waitForFinished();
    QCoreApplication::processEvents();
    return waitForRefreshedSourceFiles();
}

void ModelManagerTestHelper::resetRefreshedSourceFiles()
{
    m_lastRefreshedSourceFiles.clear();
    m_refreshHappened = false;
}

QSet<QString> ModelManagerTestHelper::waitForRefreshedSourceFiles()
{
    while (!m_refreshHappened)
        QCoreApplication::processEvents();

    return m_lastRefreshedSourceFiles;
}

void ModelManagerTestHelper::waitForFinishedGc()
{
    m_gcFinished = false;

    while (!m_gcFinished)
        QCoreApplication::processEvents();
}

void ModelManagerTestHelper::sourceFilesRefreshed(const QSet<QString> &files)
{
    m_lastRefreshedSourceFiles = files;
    m_refreshHappened = true;
}

void ModelManagerTestHelper::gcFinished()
{
    m_gcFinished = true;
}

} // namespace CppEditor::Tests
