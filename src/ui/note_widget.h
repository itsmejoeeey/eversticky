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

#ifndef NOTE_WIDGET_H
#define NOTE_WIDGET_H

#include <QMainWindow>
#include <QScrollArea>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "cache.h"
#include "note.h"
#include "note_controller.h"
#include "note_scroll_area.h"
#include "webview/note_webview.h"


class NoteWidget : public QMainWindow
{
Q_OBJECT

public:
    NoteWidget(NoteController *parent, Note* note, noteItem size);
    void testFunc();

    void updateModel(Note *note);
    void syncModel();
    Note* note;
    void bringToForeground();

    void scrollNote(QPoint amount);

public slots:
    void updateUI();

private slots:
    void deleteNote();
    void createNote();

    void titleTextUpdated(QString newText);
    void contentTextChanged(QString newText);
    void scrollToWebviewCaret(int caretY);

private:
    NoteController *parent;

    NoteWebview * noteWebview;
    NoteHeader * noteHeader;
    NoteScrollArea *noteScrollArea;

    noteItem initialNoteState;

    void setNoteSize();

    void hideFromTaskbar();
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // NOTE_WIDGET_H
