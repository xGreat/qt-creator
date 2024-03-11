// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <texteditor/fontsettings.h>
#include <texteditor/syntaxhighlighter.h>
#include <texteditor/texteditorsettings.h>

#include <QObject>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace TextEditor {

class SyntaxHighlighterRunnerPrivate;

class TEXTEDITOR_EXPORT SyntaxHighlighterRunner : public QObject
{
    Q_OBJECT
public:
    using SyntaxHighlighterCreator = std::function<SyntaxHighlighter *()>;

    SyntaxHighlighterRunner(SyntaxHighlighter *highlighter, QTextDocument *document, bool async);
    virtual ~SyntaxHighlighterRunner();

    void setExtraFormats(const QMap<int, QList<QTextLayout::FormatRange>> &formats);
    void clearExtraFormats(const QList<int> &blockNumbers);
    void clearAllExtraFormats();
    void setFontSettings(const TextEditor::FontSettings &fontSettings);
    void setLanguageFeaturesFlags(unsigned int flags);
    void setEnabled(bool enabled);
    void rehighlight();
    void reformatBlocks(int from, int charsRemoved, int charsAdded);

    QString definitionName();
    void setDefinitionName(const QString &name);

    QTextDocument *document() const { return m_document; }
    bool useGenericHighlighter() const;

    bool syntaxInfoUpdated() const { return m_syntaxInfoUpdated == SyntaxHighlighter::State::Done; }

signals:
    void highlightingFinished();

private:
    void applyFormatRanges(const QList<SyntaxHighlighter::Result> &results);
    void changeDocument(int from, int charsRemoved, int charsAdded);

    SyntaxHighlighterRunnerPrivate *d;
    QPointer<QTextDocument> m_document = nullptr;
    SyntaxHighlighter::State m_syntaxInfoUpdated = SyntaxHighlighter::State::Done;

    struct HighlightingStatus
    {
        int m_from = 0;
        int m_addedChars = 0;
        int m_current = 0;
        int m_removedChars = 0;
        int m_newFrom = 0;
        bool m_interruptionRequested = false;

        void notInterrupted(int from, int charsRemoved, int charsAdded);
        void interrupted(int from, int charsRemoved, int charsAdded);
        void applyNewFrom();
    } m_highlightingStatus;

    bool m_useGenericHighlighter = false;
    QString m_definitionName;
    std::optional<QThread> m_thread;
    TextDocumentLayout::FoldValidator m_foldValidator;
};

} // namespace TextEditor

