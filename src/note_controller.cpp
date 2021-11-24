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

#include "note_controller.h"

#include <iostream>

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "helpers/misc_helpers.h"
#include "note.h"
#include "note_formatter.h"
#include "note_sync_controller.h"
#include "settings.h"
#include "ui/note_widget.h"


NoteController::NoteController(int screens, int screenWidth, int screenHeight) : screens(screens), screenWidth(screenWidth), screenHeight(screenHeight), QObject(), state(tAuthState::UNAUTHORISED)
{
    trayIcon = new TrayIcon(this);

    login();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NoteController::periodicUpdate);
    timer->setTimerType(Qt::VeryCoarseTimer);
    // Set timer to user-defined duration. Convert from seconds to ms.
    timer->start(Settings::getUserSetting("sync_interval").toInt() * 1000);
}

NoteController::~NoteController()
{
    delete trayIcon;
}

// Creating a new note to later be created in the users' Evernote account
NoteWidget* NoteController::createNote()
{
    Note* note_model = new Note();

    // Create a random local GUID (due the fact this not has not been uploaded thus far)
    //   in the format 'LOCAL-XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX'
    // When the note has been uploaded, it will have a new GUID assigned by Evernote
    note_model->guid = "LOCAL-" + helpers::misc::random_hex_string(8) + "-" +
                                  helpers::misc::random_hex_string(4) + "-" +
                                  helpers::misc::random_hex_string(4) + "-" +
                                  helpers::misc::random_hex_string(4) + "-" +
                                  helpers::misc::random_hex_string(12);
    note_model->content = NoteFormatter::createEmptyENML();

    NoteWidget *note = new NoteWidget(this, note_model, noteItem());
    notes.push_back(note);
    noteCount++;

    return note;
}

// Create a new note where a note model instance (downloaded from Evernote) is provided
NoteWidget* NoteController::createNote(Note* note_model)
{
   NoteWidget *note = new NoteWidget(this, note_model, noteItem());
   notes.push_back(note);
   noteCount++;

   return note;
}
NoteWidget* NoteController::createNote(Note* note_model, noteItem size)
{
   NoteWidget *note = new NoteWidget(this, note_model, size);
   notes.push_back(note);
   noteCount++;

   return note;
}

void NoteController::login()
{
    if(NoteSyncController::authenticate())
    {
        state = tAuthState::AUTHORISED;

        trayIcon->updateTrayMenu();

        NoteSyncController::syncFromServer();
        showNotes();
    }
}

void NoteController::showNotes()
{
    std::vector<Note> sync_results = Cache::retrieveAllFromSyncTable();
    std::vector<Note>::iterator n = sync_results.begin();
    while(n != sync_results.end())
    {
        Note note = *n;

        // Don't bother showing note if a NoteWidget already exists for the Guid.
        if(isNoteCreated(note.guid)) {
            n++;
            continue;
        }

        queueItem item = Cache::retrieveFromQueueTable(note.guid);
        noteItem size = Cache::retrieveFromNotesTable(note.guid, this->screens, this->screenWidth, this->screenHeight);

        // If note exists in the queue table, show the changed content.
        if(item.type == "UPDATE") {
            Note *pointer_note = new Note(note.guid, note.usn, item.note.title, NoteFormatter(item.note.content).standardiseInput());
            createNote(pointer_note, size);
        }
        // Otherwise, show the original note.
        else {
            Note *pointer_note = new Note(note.guid, note.usn, note.title, NoteFormatter(note.content).standardiseInput());
            createNote(pointer_note, size);
        }
        n++;
    }

    // Also open new notes in queue that haven't been synced yet
    std::vector<queueItem> new_notes = Cache::retrieveNewFromQueueTable();
    std::vector<queueItem>::iterator p = new_notes.begin();
    while(p != new_notes.end())
    {
        Note queuedNote = (*p).note;

        // Don't bother showing note if a NoteWidget already exists for the Guid.
        if(isNoteCreated(queuedNote.guid)) {
            p++;
            continue;
        }

        Note *pointer_note = new Note(queuedNote.guid, queuedNote.usn, queuedNote.title, NoteFormatter(queuedNote.content).standardiseInput());
        createNote(pointer_note);
        p++;
    }
}

void NoteController::syncAllNoteModels()
{
    foreach(NoteWidget *note, notes) {
        note->syncModel();
    }
}

void NoteController::bringAllToFront()
{
    foreach(NoteWidget *note, notes) {
        note->bringToForeground();
    }
}

void NoteController::updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height)
{
    Cache::insertNotesTable(guid, this->screens, this->screenWidth, this->screenHeight, x, y, width, height);
}

void NoteController::periodicUpdate()
{
    // Don't update if not authenticated with Evernote.
    if(!isAuthorised())
    {
        return;
    }

    std::vector<GuidMap> changes = NoteSyncController::syncChanges();

    // If new notes have been created on sync, the local GUID needs to be updated to the the notes new
    // Guid assigned by Evernote.
    foreach(NoteWidget *note, notes) {
        foreach(GuidMap change, changes) {
            if(note->getNoteGuid() == change.local_guid)
                note->updateNoteGuid(change.official_guid);
        }
    }

    syncAllNoteModels();

    // Show any new notes
    showNotes();
}

void NoteController::logout()
{
    // Show a confirmation box before continuing if the user has unsynced changes
    if(Cache::countQueueTableRows() > 0)
    {
        QMessageBox confirmationBox;
        confirmationBox.setIcon(QMessageBox::Question);
        QIcon icon(":/icon/appicon.ico");
        confirmationBox.setWindowIcon(icon);
        confirmationBox.setWindowTitle("Confirm Logout");
        confirmationBox.setText("You have unsynced changes which will be lost unless you sync them. Do you want to continue.");
        confirmationBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

        // Return from function immediately (without logging out) if the user doesn't press 'Ok'
        if(confirmationBox.exec() != QMessageBox::Ok)
            return;
    }

    state = tAuthState::UNAUTHORISED;
    Settings::deleteAllSessionSettings();
    Cache::deleteDatabase();

    QMessageBox informationBox;
    informationBox.setIcon(QMessageBox::Information);
    QIcon icon(":/icon/appicon.ico");
    informationBox.setWindowIcon(icon);
    informationBox.setWindowTitle("Information");
    informationBox.setText("This application will now exit.");
    informationBox.setStandardButtons(QMessageBox::Ok);

    // Close application when the uer pressed 'Ok'
    if(informationBox.exec() == QMessageBox::Ok)
    {
        informationBox.close();
        QCoreApplication::quit();
    }
}

bool NoteController::isAuthorised()
{
    return (state == tAuthState::AUTHORISED);
}

bool NoteController::isNoteCreated(qevercloud::Guid noteGuid)
{
    foreach(NoteWidget *note, notes) {
        if(note->getNoteGuid() == noteGuid) {
            return true;
        }
    }

    return false;
}
