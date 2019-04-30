#ifndef CONTENT_TEXT_EDIT_H
#define CONTENT_TEXT_EDIT_H

#include <QPlainTextEdit>

#include "header_block.hpp"

class NoteUI;

class ContentTextEdit : public QTextEdit
{
Q_OBJECT
public:
    ContentTextEdit(int prefixLines, NoteUI *context);
    void attachSameScroll(HeaderBlock *headerBlock);
    void setTopPadding(int prefixLines);
    void setText(QString text);
    void clearText();
private slots:
    void scrollChanged(int newScrollPosition);
    void textChanged();

private:
    int prevLineLength;
    HeaderBlock *headerBlock;
    void resizeEvent(QResizeEvent *event) override;
    int prefixLines;
    int lineHeight;
    int lineTopPadding;
    NoteUI *parent;
    bool textSet;
};

#endif // CONTENT_TEXT_EDIT_H
