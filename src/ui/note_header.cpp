/*
 * This file is part of the EverSticky project (https://github.com/itsmejoeeey/eversticky).
 * Copyright (c) 2021 Joey Miller.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "note_header.h"

#include <iostream>

#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "note_header_text_edit.h"
#include "note_widget.h"
#include "webview/note_webview.h"


NoteHeader::NoteHeader(QWidget *context) : QWidget(context)
{
    setObjectName("NoteHeader");

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    // Create text box for note title
    headerTextEdit = new NoteHeaderTextEdit(this);
    connect(headerTextEdit, &NoteHeaderTextEdit::textUpdated, this, &NoteHeader::textUpdated);
    layout->addWidget(headerTextEdit);

    // Create line to underscore note title
    QFrame *line = new QFrame(this);
    line->setObjectName("NoteHeaderLine");
    line->setFrameShape(QFrame::HLine);
    layout->addWidget(line);
}

void NoteHeader::clearText()
{
    headerTextEdit->clear();
}

void NoteHeader::setText(QString text)
{
    headerTextEdit->setText(text);
}
