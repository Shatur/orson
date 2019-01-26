#include "processedit.h"

#include <QDebug>
#include <QRegularExpression>

ProcessEdit::ProcessEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
}

void ProcessEdit::addProcessData(const QString &text)
{
    // Check every new line for progress info
    QString processInfo = getCurrentProcessInfo();
    foreach (const QString &line, text.split(QRegularExpression("\n|\r"), QString::SkipEmptyParts)) {
        // Check if it the same task with other percentage
        if (!processInfo.isEmpty() && line.contains(processInfo)) {
            replaceLastLine(line);
        } else {
            appendPlainText(line);

            // Get new info about current task from last line
            processInfo = getCurrentProcessInfo();
        }
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

void ProcessEdit::replaceLastLine(const QString &line)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.removeSelectedText();
    cursor.insertText(line);
    setTextCursor(cursor);
}
