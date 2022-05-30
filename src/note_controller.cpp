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


NoteController::NoteController(int screens, int screenWidth, int screenHeight) : QObject(), screenHeight(screenHeight), screenWidth(screenWidth), screens(screens), state(tAuthState::UNAUTHORISED)
{
    trayIcon = new TrayIcon(this);

    login();

    // Create a timer to periodically sync notes with Evernote.
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

    connect(note, &QWidget::destroyed, this, [this, note](){
        noteDestroyed(note);
    });

    return note;
}

// Create a new note where a note model instance (downloaded from Evernote) is provided
NoteWidget* NoteController::createNote(Note* note_model)
{
   NoteWidget *note = new NoteWidget(this, note_model, noteItem());
   notes.push_back(note);
   noteCount++;

   connect(note, &QWidget::destroyed, this, [this, note](){
       noteDestroyed(note);
   });

   return note;
}
NoteWidget* NoteController::createNote(Note* note_model, noteItem size)
{
   NoteWidget *note = new NoteWidget(this, note_model, size);
   notes.push_back(note);
   noteCount++;

   connect(note, &QWidget::destroyed, this, [this, note](){
       noteDestroyed(note);
   });

   return note;
}

void NoteController::noteDestroyed(NoteWidget *note)
{
    auto it = std::find(notes.begin(), notes.end(), note);
    // Remove the destroyed widget from the vector if it exists
    if (it != notes.end())
        notes.erase(it);
}

void NoteController::login()
{
    // Sets the initial authentication state by checking the saved session settings.
    if(Settings::getSessionSetting("auth_token").isEmpty() ||
       Settings::getSessionSetting("notestore_url").isEmpty() ||
       Settings::getSessionSetting("username").isEmpty())
    {
        // Do OAuth login
        state = NoteSyncController::login();
    } else {
        state = tAuthState::AUTHORISED;
    }

    trayIcon->updateTrayMenu();

    // If the user is now authenticated, create a sync controller with their credentials.
    if(isAuthorised())
    {
        noteSyncController = new NoteSyncController(
                    Settings::getSessionSetting("auth_token"),
                    Settings::getSessionSetting("notebook_guid"),
                    Settings::getSessionSetting("notestore_url")
        );
        connect(noteSyncController, &NoteSyncController::authInvalid, this, [this](){
            // Delete session credentials and show OAuth login.
            softLogout();
            login();
        });

        // Do the first update immediately after login.
        periodicUpdate();
    }
}

// Logout and forget username and all user-data (including unsynced note changes).
// --> This is used when a user chooses to logout of the application (from the tray icon).
void NoteController::hardLogout()
{
    state = tAuthState::UNAUTHORISED;
    trayIcon->updateTrayMenu();

    closeAllNotes();

    Settings::deleteAllSessionSettings();
    Cache::deleteDatabase();
}

// Logout but remember username and keep user-data (including unsynced note changes).
// --> This is used when a users auth_token expires. This gives the user a chance to login
//     again without losing all of their changes.
void NoteController::softLogout()
{
    state = tAuthState::UNAUTHORISED;
    trayIcon->updateTrayMenu();

    closeAllNotes();

    Settings::deleteCurrentSessionSettings();
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

        std::optional<queueItem> item = Cache::retrieveFromQueueTable(note.guid);
        noteItem size = Cache::retrieveFromNotesTable(note.guid, this->screens, this->screenWidth, this->screenHeight);

        if(item) {
            if((*item).type == "UPDATE") {
                // If note exists in the queue table and has been updated, show the changed content.
                Note itemNote = (*item).note;
                Note *pointer_note = new Note(note.guid, note.usn, itemNote.title, NoteFormatter(itemNote.content).standardiseInput());
                createNote(pointer_note, size);
            } else {
                // Otherwise do nothing.
            }
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

void NoteController::closeAllNotes()
{
    std::vector<NoteWidget*>::iterator it = notes.begin();
    while(it != notes.end())
    {
        // Close the NoteWidget window
        (*it)->close();

        // Erase makes the interator invalid and returns the next iterator
        it = notes.erase(it);
    }
}

void NoteController::updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height)
{
    Cache::insertNotesTable(guid, this->screens, this->screenWidth, this->screenHeight, x, y, width, height);
}

void NoteController::periodicUpdate()
{
    // Don't update if not authenticated.
    if(!isAuthorised())
    {
        return;
    }

    std::vector<GuidMap> changes = noteSyncController->syncChanges();

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

void NoteController::showLogoutDialog()
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

    hardLogout();

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
