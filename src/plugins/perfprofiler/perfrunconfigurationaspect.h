// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <projectexplorer/runconfiguration.h>

namespace PerfProfiler::Internal {

class PerfRunConfigurationAspect final : public ProjectExplorer::GlobalOrProjectAspect
{
public:
    explicit PerfRunConfigurationAspect(ProjectExplorer::Target *target);
};

} // PerfProfiler::Internal
