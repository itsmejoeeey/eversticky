#include "header_text_edit.hpp"

// Standard libs
#include <iostream>
// QT Libs
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>


HeaderTextEdit::HeaderTextEdit(QWidget *parent, HeaderBlock *parentController) : QPlainTextEdit(parent)
{
    this->parent = parentController;
    this->installEventFilter(this);
    this->setObjectName("title_editor");
    this->setStyleSheet("border: 1px solid red");
    this->setFixedHeight(50);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()));

}

void HeaderTextEdit::textChanged() {
    //int height = this->document()->documentLayout()->documentSize().height() + 20;
    QTextBlock block = this->document()->begin();
    int height = this->document()->documentLayout()->blockBoundingRect(block).height();
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);

    int lines = 1;
    while(cursor.positionInBlock() > 0) {
        cursor.movePosition(QTextCursor::Up);
        lines++;
    }
    //cursor.movePosition(QTextCursor::EndOfBlock);

    int lineHeight = this->document()->begin().blockFormat().lineHeight();

    this->setFixedHeight(height + 15);
    this->updateGeometry();
    //parent->updateLeftColumnLayout();
    parent->textChanged();
}

bool HeaderTextEdit::eventFilter(QObject *object, QKeyEvent *event) {
    if(event->key() == Qt::Key_Return) {
        return false;
    } else if(event->key() == Qt::Key_Backspace) {

    }
    this->textChanged();
    return this->eventFilter(object, event);

}

void HeaderTextEdit::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Return) {
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}


void HeaderTextEdit::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    textChanged();
}


void HeaderTextEdit::setText(QString text) {
    this->clear();
    this->insertPlainText(text);
}

QString HeaderTextEdit::getText() {
    return this->toPlainText();
}
