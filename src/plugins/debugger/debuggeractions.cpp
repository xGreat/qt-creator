// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "debuggeractions.h"

#include "commonoptionspage.h"
#include "debuggericons.h"
#include "debuggerinternalconstants.h"
#include "debuggertr.h"
#include "gdb/gdbsettings.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>

#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QDebug>
#include <QGuiApplication>

using namespace Utils;

namespace Debugger::Internal {

// DebuggerSettings

DebuggerSettings &settings()
{
    static DebuggerSettings settings;
    return settings;
}

DebuggerSettings::DebuggerSettings() :
    useAlternatingRowColors{commonSettings().useAlternatingRowColors},
    useAnnotationsInMainEditor{commonSettings().useAnnotationsInMainEditor},
    useToolTipsInMainEditor{commonSettings().useToolTipsInMainEditor},
    closeSourceBuffersOnExit{commonSettings().closeSourceBuffersOnExit},
    closeMemoryBuffersOnExit{commonSettings().closeMemoryBuffersOnExit},
    raiseOnInterrupt{commonSettings().raiseOnInterrupt},
    breakpointsFullPathByDefault{commonSettings().breakpointsFullPathByDefault},
    warnOnReleaseBuilds{commonSettings().warnOnReleaseBuilds},
    maximalStackDepth{commonSettings().maximalStackDepth},

    fontSizeFollowsEditor{commonSettings().fontSizeFollowsEditor},
    switchModeOnExit{commonSettings().switchModeOnExit},
    showQmlObjectTree{commonSettings().showQmlObjectTree},
    stationaryEditorWhileStepping{commonSettings().stationaryEditorWhileStepping},
    forceLoggingToConsole{commonSettings().forceLoggingToConsole},

    sourcePathMap{commonSettings().sourcePathMap},
    registerForPostMortem{*commonSettings().registerForPostMortem},

    gdbWatchdogTimeout{gdbSettings().gdbWatchdogTimeout},
    skipKnownFrames{gdbSettings().skipKnownFrames},
    useMessageBoxForSignals{gdbSettings().useMessageBoxForSignals},
    adjustBreakpointLocations{gdbSettings().adjustBreakpointLocations},
    useDynamicType{gdbSettings().useDynamicType},
    loadGdbInit{gdbSettings().loadGdbInit},
    loadGdbDumpers{gdbSettings().loadGdbDumpers},
    intelFlavor{gdbSettings().intelFlavor},
    usePseudoTracepoints{gdbSettings().usePseudoTracepoints},
    useIndexCache{gdbSettings().useIndexCache},
    gdbStartupCommands{gdbSettings().gdbStartupCommands},
    gdbPostAttachCommands{gdbSettings().gdbPostAttachCommands},
    targetAsync{gdbSettings().targetAsync},
    autoEnrichParameters{gdbSettings().autoEnrichParameters},
    breakOnThrow{gdbSettings().breakOnThrow},
    breakOnCatch{gdbSettings().breakOnCatch},
    breakOnWarning{gdbSettings().breakOnWarning},
    breakOnFatal{gdbSettings().breakOnFatal},
    breakOnAbort{gdbSettings().breakOnAbort},
    enableReverseDebugging{gdbSettings().enableReverseDebugging},
    multiInferior{gdbSettings().multiInferior}
{
    const QString debugModeGroup("DebugMode");
    const QString cdbSettingsGroup("CDB2");

    settingsDialog.setLabelText(Tr::tr("Configure Debugger..."));

    /*
    groupBoxPluginDebugging = new QGroupBox(q);
    groupBoxPluginDebugging->setTitle(Tr::tr(
        "Behavior of Breakpoint Setting in Plugins"));

    radioButtonAllPluginBreakpoints = new QRadioButton(groupBoxPluginDebugging);
    radioButtonAllPluginBreakpoints->setText(Tr::tr(
        "Always try to set breakpoints in plugins automatically"));
    radioButtonAllPluginBreakpoints->setToolTip(Tr::tr(
        "This is the slowest but safest option."));

    radioButtonSelectedPluginBreakpoints = new QRadioButton(groupBoxPluginDebugging);
    radioButtonSelectedPluginBreakpoints->setText(Tr::tr(
        "Try to set breakpoints in selected plugins"));

    radioButtonNoPluginBreakpoints = new QRadioButton(groupBoxPluginDebugging);
    radioButtonNoPluginBreakpoints->setText(Tr::tr(
        "Never set breakpoints in plugins automatically"));

    lineEditSelectedPluginBreakpointsPattern = new QLineEdit(groupBoxPluginDebugging);

    labelSelectedPluginBreakpoints = new QLabel(groupBoxPluginDebugging);
    labelSelectedPluginBreakpoints->setText(Tr::tr(
        "Matching regular expression: "));
    */

    //
    // View
    //
    alwaysAdjustColumnWidths.setLabelText(Tr::tr("Always Adjust View Column Widths to Contents"));
    alwaysAdjustColumnWidths.setSettingsKey(debugModeGroup, "AlwaysAdjustColumnWidths");
    alwaysAdjustColumnWidths.setDefaultValue(true);

    logTimeStamps.setLabelText(Tr::tr("Log Time Stamps"));
    logTimeStamps.setSettingsKey(debugModeGroup, "LogTimeStamps");

    autoDerefPointers.setLabelText(Tr::tr("Dereference Pointers Automatically"));
    autoDerefPointers.setDefaultValue(true);
    autoDerefPointers.setSettingsKey(debugModeGroup, "AutoDerefPointers");
    autoDerefPointers.setToolTip(
        "<p>"
        + Tr::tr("This switches the Locals and Expressions views to "
                 "automatically dereference pointers. This saves a level in the "
                 "tree view, but also loses data for the now-missing intermediate "
                 "level."));

    //
    // Cdb Options
    //

    cdbAdditionalArguments.setSettingsKey(cdbSettingsGroup, "AdditionalArguments");
    cdbAdditionalArguments.setDisplayStyle(StringAspect::LineEditDisplay);
    cdbAdditionalArguments.setLabelText(Tr::tr("Additional arguments:"));

    cdbSymbolPaths.setSettingsKey(cdbSettingsGroup, "SymbolPaths");
    cdbSourcePaths.setSettingsKey(cdbSettingsGroup, "SourcePaths");

    cdbBreakEvents.setSettingsKey(cdbSettingsGroup, "BreakEvent");
    cdbBreakOnCrtDbgReport.setSettingsKey(cdbSettingsGroup, "BreakOnCrtDbgReport");
    cdbBreakOnCrtDbgReport.setLabelText(msgSetBreakpointAtFunction(Constants::CRT_DEBUG_REPORT));
    cdbBreakOnCrtDbgReport.setToolTip(msgSetBreakpointAtFunctionToolTip(Constants::CRT_DEBUG_REPORT,
        Tr::tr("Catches runtime error messages caused by assert(), for example.")));

    useCdbConsole.setSettingsKey(cdbSettingsGroup, "CDB_Console");
    useCdbConsole.setToolTip("<html><head/><body><p>" + Tr::tr(
        "Uses CDB's native console for console applications. "
        "This overrides the setting in Environment > System. "
        "The native console does not prompt on application exit. "
        "It is suitable for diagnosing cases in which the application does not "
        "start up properly in the configured console and the subsequent attach fails.")
        + "</p></body></html>");
    useCdbConsole.setLabelText(Tr::tr("Use CDB &console"));

    cdbBreakPointCorrection.setSettingsKey(cdbSettingsGroup, "BreakpointCorrection");
    cdbBreakPointCorrection.setDefaultValue(true);
    cdbBreakPointCorrection.setToolTip("<html><head/><body><p>" + Tr::tr(
        "Attempts to correct the location of a breakpoint based on file and line number should "
        "it be in a comment or in a line for which no code is generated. "
        "The correction is based on the code model.") + "</p></body></html>");
    cdbBreakPointCorrection.setLabelText(Tr::tr("Correct breakpoint location"));

    cdbUsePythonDumper.setSettingsKey(cdbSettingsGroup, "UsePythonDumper");
    cdbUsePythonDumper.setDefaultValue(true);
    cdbUsePythonDumper.setLabelText(Tr::tr("Use Python dumper"));

    firstChanceExceptionTaskEntry.setSettingsKey(cdbSettingsGroup, "FirstChanceExceptionTaskEntry");
    firstChanceExceptionTaskEntry.setDefaultValue(true);
    firstChanceExceptionTaskEntry.setLabelText(Tr::tr("First chance exceptions"));

    secondChanceExceptionTaskEntry.setSettingsKey(cdbSettingsGroup, "SecondChanceExceptionTaskEntry");
    secondChanceExceptionTaskEntry.setDefaultValue(true);
    secondChanceExceptionTaskEntry.setLabelText(Tr::tr("Second chance exceptions"));

    ignoreFirstChanceAccessViolation.setSettingsKey(cdbSettingsGroup, "IgnoreFirstChanceAccessViolation");
    ignoreFirstChanceAccessViolation.setLabelText(Tr::tr("Ignore first chance access violations"));

    //
    // Locals & Watchers
    //
    showStdNamespace.setSettingsKey(debugModeGroup, "ShowStandardNamespace");
    showStdNamespace.setDefaultValue(true);
    showStdNamespace.setDisplayName(Tr::tr("Show \"std::\" Namespace in Types"));
    showStdNamespace.setLabelText(Tr::tr("Show \"std::\" namespace in types"));
    showStdNamespace.setToolTip(
        "<p>" + Tr::tr("Shows \"std::\" prefix for types from the standard library."));

    showQtNamespace.setSettingsKey(debugModeGroup, "ShowQtNamespace");
    showQtNamespace.setDefaultValue(true);
    showQtNamespace.setDisplayName(Tr::tr("Show Qt's Namespace in Types"));
    showQtNamespace.setLabelText(Tr::tr("Show Qt's namespace in types"));
    showQtNamespace.setToolTip("<p>"
                               + Tr::tr("Shows Qt namespace prefix for Qt types. This is only "
                                        "relevant if Qt was configured with \"-qtnamespace\"."));

    showQObjectNames.setSettingsKey(debugModeGroup, "ShowQObjectNames2");
    showQObjectNames.setDefaultValue(true);
    showQObjectNames.setDisplayName(Tr::tr("Show QObject names if available"));
    showQObjectNames.setLabelText(Tr::tr("Show QObject names if available"));
    showQObjectNames.setToolTip(
        "<p>"
        + Tr::tr("Displays the objectName property of QObject based items. "
                 "Note that this can negatively impact debugger performance "
                 "even if no QObjects are present."));

    sortStructMembers.setSettingsKey(debugModeGroup, "SortStructMembers");
    sortStructMembers.setDisplayName(Tr::tr("Sort Members of Classes and Structs Alphabetically"));
    sortStructMembers.setLabelText(Tr::tr("Sort members of classes and structs alphabetically"));
    sortStructMembers.setDefaultValue(true);

    //
    // DebuggingHelper
    //
    useDebuggingHelpers.setSettingsKey(debugModeGroup, "UseDebuggingHelper");
    useDebuggingHelpers.setDefaultValue(true);
    useDebuggingHelpers.setLabelText(Tr::tr("Use Debugging Helpers"));

    useCodeModel.setSettingsKey(debugModeGroup, "UseCodeModel");
    useCodeModel.setDefaultValue(true);
    useCodeModel.setLabelText(Tr::tr("Use code model"));
    useCodeModel.setToolTip(
        "<p>"
        + Tr::tr("Selecting this causes the C++ Code Model being asked "
                 "for variable scope information. This might result in slightly faster "
                 "debugger operation but may fail for optimized code."));

    showThreadNames.setSettingsKey(debugModeGroup, "ShowThreadNames");
    showThreadNames.setLabelText(Tr::tr("Display thread names"));
    showThreadNames.setToolTip("<p>" + Tr::tr("Displays names of QThread based threads."));

    //
    // Breakpoints
    //
    synchronizeBreakpoints.setLabelText(Tr::tr("Synchronize Breakpoints"));

    extraDumperCommands.setSettingsKey(debugModeGroup, "GdbCustomDumperCommands");
    extraDumperCommands.setDisplayStyle(StringAspect::TextEditDisplay);
    extraDumperCommands.setUseGlobalMacroExpander();
    extraDumperCommands.setToolTip("<html><head/><body><p>"
                        + Tr::tr("Python commands entered here will be executed after built-in "
                             "debugging helpers have been loaded and fully initialized. You can "
                             "load additional debugging helpers or modify existing ones here.")
                        + "</p></body></html>");

    extraDumperFile.setSettingsKey(debugModeGroup, "ExtraDumperFile");
    extraDumperFile.setDisplayName(Tr::tr("Extra Debugging Helpers"));
    // Label text is intentional empty in the GUI.
    extraDumperFile.setToolTip(Tr::tr("Path to a Python file containing additional data dumpers."));

    autoQuit.setSettingsKey(debugModeGroup, "AutoQuit");
    autoQuit.setLabelText(Tr::tr("Automatically Quit Debugger"));

    useToolTipsInLocalsView.setSettingsKey(debugModeGroup, "UseToolTipsInLocalsView");
    useToolTipsInLocalsView.setLabelText(Tr::tr("Use Tooltips in Locals View when Debugging"));
    useToolTipsInLocalsView.setToolTip("<p>"
                                       + Tr::tr("Enables tooltips in the locals "
                                                "view during debugging."));

    useToolTipsInBreakpointsView.setSettingsKey(debugModeGroup, "UseToolTipsInBreakpointsView");
    useToolTipsInBreakpointsView.setLabelText(Tr::tr("Use Tooltips in Breakpoints View when Debugging"));
    useToolTipsInBreakpointsView.setToolTip("<p>"
                                            + Tr::tr("Enables tooltips in the breakpoints "
                                                     "view during debugging."));

    useToolTipsInStackView.setSettingsKey(debugModeGroup, "UseToolTipsInStackView");
    useToolTipsInStackView.setLabelText(Tr::tr("Use Tooltips in Stack View when Debugging"));
    useToolTipsInStackView.setToolTip("<p>"
                                      + Tr::tr("Enables tooltips in the stack "
                                               "view during debugging."));
    useToolTipsInStackView.setDefaultValue(true);

    allPluginBreakpoints.setSettingsKey(debugModeGroup, "AllPluginBreakpoints");
    allPluginBreakpoints.setDefaultValue(true);

    selectedPluginBreakpoints.setSettingsKey(debugModeGroup, "SelectedPluginBreakpoints");

    noPluginBreakpoints.setSettingsKey(debugModeGroup, "NoPluginBreakpoints");

    selectedPluginBreakpointsPattern.setSettingsKey(debugModeGroup, "SelectedPluginBreakpointsPattern");
    selectedPluginBreakpointsPattern.setDefaultValue(QString(".*"));

    displayStringLimit.setSettingsKey(debugModeGroup, "DisplayStringLimit");
    displayStringLimit.setDefaultValue(300);
    displayStringLimit.setSpecialValueText(Tr::tr("<unlimited>"));
    displayStringLimit.setRange(20, 10000);
    displayStringLimit.setSingleStep(10);
    displayStringLimit.setLabelText(Tr::tr("Display string length:"));
    displayStringLimit.setToolTip(
        "<p>"
        + Tr::tr("The maximum length of string entries in the "
                 "Locals and Expressions views. Longer than that are cut off "
                 "and displayed with an ellipsis attached."));

    maximalStringLength.setSettingsKey(debugModeGroup, "MaximalStringLength");
    maximalStringLength.setDefaultValue(10000);
    maximalStringLength.setSpecialValueText(Tr::tr("<unlimited>"));
    maximalStringLength.setRange(20, 10000000);
    maximalStringLength.setSingleStep(20);
    maximalStringLength.setLabelText(Tr::tr("Maximum string length:"));
    maximalStringLength.setToolTip(
        "<p>"
        + Tr::tr("The maximum length for strings in separated windows. "
                 "Longer strings are cut off and displayed with an ellipsis attached."));

    defaultArraySize.setSettingsKey(debugModeGroup, "DefaultArraySize");
    defaultArraySize.setDefaultValue(100);
    defaultArraySize.setRange(10, 1000000000);
    defaultArraySize.setSingleStep(100);
    defaultArraySize.setLabelText(Tr::tr("Default array size:"));
    defaultArraySize.setToolTip("<p>"
                                + Tr::tr("The number of array elements requested when expanding "
                                         "entries in the Locals and Expressions views."));

    expandStack.setLabelText(Tr::tr("Reload Full Stack"));

    createFullBacktrace.setLabelText(Tr::tr("Create Full Backtrace"));

    //
    // QML Tools
    //
    const QString qmlInspectorGroup = "QML.Inspector";
    showAppOnTop.setSettingsKey(qmlInspectorGroup, "QmlInspector.ShowAppOnTop");


    // Page 4
    page4.registerAspect(&useDebuggingHelpers);
    page4.registerAspect(&useCodeModel);
    page4.registerAspect(&showThreadNames);
    page4.registerAspect(&showStdNamespace);
    page4.registerAspect(&showQtNamespace);
    page4.registerAspect(&extraDumperFile);
    page4.registerAspect(&extraDumperCommands);
    page4.registerAspect(&showQObjectNames);
    page4.registerAspect(&displayStringLimit);
    page4.registerAspect(&maximalStringLength);
    page4.registerAspect(&defaultArraySize);

    // Page 5
    page5.registerAspect(&cdbAdditionalArguments);
    page5.registerAspect(&cdbBreakEvents);
    page5.registerAspect(&cdbBreakOnCrtDbgReport);
    page5.registerAspect(&useCdbConsole);
    page5.registerAspect(&cdbBreakPointCorrection);
    page5.registerAspect(&cdbUsePythonDumper);
    page5.registerAspect(&firstChanceExceptionTaskEntry);
    page5.registerAspect(&secondChanceExceptionTaskEntry);
    page5.registerAspect(&ignoreFirstChanceAccessViolation);

    // Page 6
    page6.registerAspect(&cdbSymbolPaths);
    page6.registerAspect(&cdbSourcePaths);

    // Pageless
    all.registerAspect(&autoDerefPointers);
    all.registerAspect(&useToolTipsInLocalsView);
    all.registerAspect(&alwaysAdjustColumnWidths);
    all.registerAspect(&useToolTipsInBreakpointsView);
    all.registerAspect(&useToolTipsInStackView);
    all.registerAspect(&logTimeStamps);
    all.registerAspect(&sortStructMembers);

    // Collect all
    all.registerAspects(page1);
    all.registerAspects(page4);
    all.registerAspects(page5);
    all.registerAspects(page6);

    all.forEachAspect([](BaseAspect *aspect) {
        aspect->setAutoApply(false);
        // FIXME: Make the positioning part of the LayoutBuilder later
        if (auto boolAspect = dynamic_cast<BoolAspect *>(aspect))
            boolAspect->setLabelPlacement(BoolAspect::LabelPlacement::AtCheckBox);
    });

    all.readSettings();

    QObject::connect(Core::ICore::instance(), &Core::ICore::saveSettingsRequested,
                     &all, &AspectContainer::writeSettings);

}

QString DebuggerSettings::dump()
{
    QStringList msg;
    settings().all.forEachAspect([&msg](BaseAspect *aspect) {
        QString key = aspect->settingsKey();
        if (!key.isEmpty()) {
            const int pos = key.indexOf('/');
            if (pos >= 0)
                key = key.mid(pos);
            const QString current = aspect->variantValue().toString();
            const QString default_ = aspect->defaultVariantValue().toString();
            QString setting = key + ": " + current + "  (default: " + default_ + ')';
            if (current != default_)
                setting +=  "  ***";
            msg << setting;
        }
    });
    msg.sort();
    return "Debugger settings:\n" + msg.join('\n');
}

} // Debugger::Internal

