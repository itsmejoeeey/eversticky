#include "header_block.hpp"

// Standard libs
#include <iostream>
// QT Libs
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
// Project libs
#include "content_text_edit.hpp"
#include "header_text_edit.hpp"
#include "note_ui.hpp"


HeaderBlock::HeaderBlock(ContentTextEdit *parent, NoteUI *context) : QWidget(parent)
{
    textSet = false;
    this->parent = parent;
    this->context = context;
    this->move(0, 0);

    main_layout = new QHBoxLayout();
    this->setLayout(main_layout);
    //this->setStyleSheet("background-color:grey;");

    left_column_frame = new QWidget();

    left_column_layout = new QVBoxLayout();
    left_column_frame->setLayout(left_column_layout);
    header_text = new HeaderTextEdit(left_column_frame, this);
    left_column_layout->addWidget(header_text);
    QFrame *line = new QFrame(left_column_frame);
    line->setObjectName("line");
    line->setFrameShape(QFrame::HLine);
    left_column_layout->addWidget(line);

    main_layout->addWidget(left_column_frame);

    updateLeftColumnLayout();
}

void HeaderBlock::updateLeftColumnLayout() {
    left_column_layout->update();
    left_column_frame->updateGeometry();
    this->adjustSize();

    main_layout->update();
    this->parent->setTopPadding(this->height());
}


void HeaderBlock::setScrollPosition(int newScrollPosition) {
    int scrollPosition = -1 * newScrollPosition;
    this->move(0, scrollPosition);
}

void HeaderBlock::resizeEvent(QResizeEvent *event) {
    int parentWidth = parent->size().width();
    this->setFixedWidth(parentWidth - 20);
}

int HeaderBlock::getHeight() {
    return this->size().height();
}

void HeaderBlock::setText(QString text) {
    textSet = false;
    header_text->setText(text);
    textSet = true;
}

void HeaderBlock::clearText() {
    textSet = false;
    header_text->clear();
}

QString HeaderBlock::getText() {
    if(header_text) {
        QString text = header_text->getText();
        return text;
    }
    return NULL;
}

void HeaderBlock::textChanged() {
    if(textSet) {
        this->context->titleTextChanged();
    }

}
