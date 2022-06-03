/*
 * This file is part of the EverSticky project (https://eversticky.joeeey.com).
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

#include "note_header_text_edit.h"

#include <iostream>

#include <QAbstractTextDocumentLayout>
#include <QMenu>
#include <QTextBlock>


NoteHeaderTextEdit::NoteHeaderTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    setObjectName("NoteHeaderTextEdit");

    setFixedHeight(50);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, &QPlainTextEdit::textChanged, this, &NoteHeaderTextEdit::textChanged);
}

void NoteHeaderTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    QAction *lastSeparator;
    foreach (QAction *action, menu->actions()) {
        // Remove 'shortcut' text from action to match context menu of note body
        QString actionText = action->text();
        action->setText(actionText.split("\t").at(0));

        action->setIcon(QIcon());

        if(action->isSeparator()) {
            lastSeparator = action;
        }
    }
    // Remove the last separator (between 'Delete' and 'Select All') to match context menu of note body
    menu->removeAction(lastSeparator);

    menu->exec(event->globalPos());
    delete menu;
}

void NoteHeaderTextEdit::textChanged()
{
    resizeToFitContent();

    emit textUpdated(getText());
}

void NoteHeaderTextEdit::resizeToFitContent()
{
    // Calculate and resize to new height of header text
    const QTextBlock block = document()->begin();
    const int height = document()->documentLayout()->blockBoundingRect(block).height();
    setFixedHeight(height + 15);
    updateGeometry();
}

void NoteHeaderTextEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return) {
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}

void NoteHeaderTextEdit::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    resizeToFitContent();
}

void NoteHeaderTextEdit::setText(QString text)
{
    setPlainText(text);
    resizeToFitContent();
}

QString NoteHeaderTextEdit::getText()
{
    return toPlainText();
}
