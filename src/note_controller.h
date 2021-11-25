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
#include "ui/tray_icon.h"


class NoteWidget;

class NoteController : public QObject
{
Q_OBJECT

public:
    NoteController(int screens, int screenWidth, int screenHeight);
    ~NoteController();

    NoteWidget* createNote();
    NoteWidget* createNote(Note *note);
    NoteWidget* createNote(Note* note_model, noteItem size);

    bool isAuthorised();

    void syncAllNoteModels();

    void bringAllToFront();
    void closeAllNotes();
    void updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height);

    void logoutPrepare();
    void showLogoutDialog();

public slots:
    void login();
    void showNotes();
    void periodicUpdate();

private:
    tAuthState state;

    NoteSyncController* noteSyncController;
    TrayIcon *trayIcon;

    std::vector<NoteWidget*> notes;
    int noteCount;

    int screens;
    int screenWidth;
    int screenHeight;

    void hardLogout();
    void softLogout();

    bool isNoteCreated(qevercloud::Guid noteGuid);
};

#endif // NOTECONTROLLER_H
