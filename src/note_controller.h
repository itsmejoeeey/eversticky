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

#ifndef NOTECONTROLLER_H
#define NOTECONTROLLER_H

#include <vector>

#include <QEverCloud.h>
#include <QMainWindow>
#include <QSettings>
#include <QObject>
#include <QWidget>

#include "cache.h"
#include "note.h"
#include "note_sync_controller.h"


class NoteWidget;

class NoteController : public QObject
{
Q_OBJECT

public:
    NoteController(int screens, int screenWidth, int screenHeight);
    NoteWidget* createNote();
    NoteWidget* createNote(Note *note);
    NoteWidget* createNote(Note* note_model, noteItem size);

    bool isAuthorised();

    void syncFromServer();
    void syncAllNoteModels();

    void bringAllToFront();
    void updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height);

    void logoutPrepare();
    void logout();

public slots:
    bool authenticate();
    void getNotebookGUID();
    void showNotes();
    void syncChanges();
    void periodicUpdate();

private:
    tAuthState state;

    std::vector<NoteWidget*> notes;
    int noteCount;
    qevercloud::Guid notebookGUID;

    int screens;
    int screenWidth;
    int screenHeight;
};

#endif // NOTECONTROLLER_H
