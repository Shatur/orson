#include "processedit.h"

#include <QDebug>
#include <QRegularExpression>

ProcessEdit::ProcessEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

void ProcessEdit::addProcessData(const QString &text)
{
    // Check every new line for progress info
    foreach (const QString &line, formatProcessData(text)) {
        const QString processInfo = getCurrentProcessInfo();
        // Check if it the same task with other percentage
        if (!processInfo.isEmpty() && line.contains(processInfo))
            replaceLastLine(line);
        else
            append(line);
    }
}

// Get text from the first bracket to two spaces
QString ProcessEdit::getCurrentProcessInfo()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.select(QTextCursor::LineUnderCursor);

    const int firstIndex = cursor.selectedText().indexOf(')') + 2;
    if (firstIndex == -1)
        return QString();

    const int lastIndex = cursor.selectedText().indexOf("  ");
    if (lastIndex == -1)
        return QString();

    return cursor.selectedText().mid(firstIndex, lastIndex - firstIndex);
}

QStringList ProcessEdit::formatProcessData(QString text)
{
    // Parse ANSI color codes
    text.replace("\r\n\u001B[0m", "\u001B[0m\r\n"); // Swap to display colors correctly
    text.replace("\u001B[1;34m::", R"(<span style="color: DodgerBlue">::</span>)");
    text.replace("\u001B[1;33m", R"(<span style="font-weight: bold; color: DarkOrange">)");
    text.replace("\u001B[0;1m", R"(<span style="font-weight: bold">)");
    text.replace("\u001B[1;31m", R"(<span style="font-weight: bold; color: Red">)");
    text.replace("\u001B[0m", "</span>");

    QStringList lines = text.split(QRegularExpression("\r\n|\r"));
    if (lines.last().isEmpty())
        lines.removeLast();

    return lines;
}

void ProcessEdit::replaceLastLine(const QString &line)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.removeSelectedText();
    cursor.insertText(line);
    setTextCursor(cursor);
}
