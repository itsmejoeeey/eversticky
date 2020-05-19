#include "content_text_edit.hpp"

// Standard libs
#include <iostream>
// QT libs
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDropEvent>
#include <QScrollBar>
#include <QTextBlock>
// Project libs
#include "note_ui.hpp"


ContentTextEdit::ContentTextEdit(int prefixLines, NoteUI *context) : QTextEdit()
{
    textSet = false;
    this->setObjectName("content_editor");
    this->setAcceptDrops(true);
    this->parent = context;
    //this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->prefixLines = prefixLines;

    //connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollChanged(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()));

    prevLineLength = this->document()->blockCount();

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setAcceptRichText(false);

    //this->setMinimumHeight(1000);
    //this->resize(QSize(this->width(),1000));
    //this->setBaseSize(1000,1000);

    //setTopPadding(1000);

}

/*
void ContentTextEdit::attachSameScroll(HeaderBlock *headerBlock) {
    this->headerBlock = headerBlock;

    setTopPadding(this->headerBlock->getHeight());
}

void ContentTextEdit::scrollChanged(int newScrollPosition) {
    if(headerBlock != NULL) {
        headerBlock->setScrollPosition(newScrollPosition);
    }

}

void ContentTextEdit::resizeEvent(QResizeEvent *event) {
    QTextEdit::resizeEvent(event);

    if(headerBlock != NULL) {
        headerBlock->resizeEvent(event);
    }

}

void ContentTextEdit::setTopPadding(int topPadding) {
    QTextFrameFormat format = this->document()->rootFrame()->frameFormat();
    format.setTopMargin(topPadding);
    this->document()->rootFrame()->setFrameFormat(format);;

}
*/

void ContentTextEdit::setText(QString text) {
    textSet = false;
    this->clear();
    this->setPlainText(text);

    //setTopPadding(this->headerBlock->getHeight());

    textSet = true;
}

void ContentTextEdit::textChanged() {
    if(textSet) {
        QString newText = this->toPlainText();
        this->parent->contentTextChanged(newText);
    }

}

void ContentTextEdit::clearText() {
    textSet = false;
    this->clear();
}
