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

#include <QEverCloudOAuth.h>

#include "helpers/misc_helpers.h"
#include "note.h"
#include "note_formatter.h"
#include "settings.h"
#include "ui/note_widget.h"
#include "ui/tray_icon.h"


NoteController::NoteController(int screens, int screenWidth, int screenHeight) : screens(screens), screenWidth(screenWidth), screenHeight(screenHeight), QObject(), state(tAuthState::UNAUTHORISED)
{
    new TrayIcon(this);

    if(authenticate()) {
        getNotebookGUID();
        showNotes();
    }

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NoteController::periodicUpdate);
    timer->setTimerType(Qt::VeryCoarseTimer);
    // Set timer to user-defined duration. Convert from seconds to ms.
    timer->start(Settings::getUserSetting("sync_interval").toInt() * 1000);
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


/*
 * For testing purposes only
*/

bool NoteController::authenticate()
{
    bool authenticated = false;

    if(!(Settings::getSessionSetting("auth_token") == "" ||
         Settings::getSessionSetting("notestore_url") == "" ||
         Settings::getSessionSetting("username") == "" )) {
        try {
            qevercloud::UserStore user_store(QString::fromStdString(API_HOST), Settings::getSessionSetting("auth_token"));
            qevercloud::User user = user_store.getUser();
            authenticated = true;
        } catch(qevercloud::EDAMUserException) {
            logoutPrepare();
        } catch(qevercloud::EDAMSystemException) {
            logoutPrepare();
        }
    }

    if(!authenticated) {
        // Test OAuth
        qevercloud::EvernoteOAuthDialog dialog(QString::fromStdString(API_KEY), QString::fromStdString(API_SECRET), QString::fromStdString(API_HOST));
        dialog.setWindowTitle("Log in to Evernote | eversticky");
        if (dialog.exec() != QDialog::Accepted) {
            // OAuth login failed
            QMessageBox::critical(0, "NotePoster", "Login failed.\n" + dialog.oauthError());

            state = tAuthState::UNAUTHORISED;
            return false;
        }

        QString auth_token = dialog.oauthResult().authenticationToken;
        QString notestore_url = dialog.oauthResult().noteStoreUrl;

        qevercloud::UserStore user_store(QString::fromStdString(API_HOST), auth_token);
        qevercloud::User user = user_store.getUser();
        QString username = user.username;

        Settings::setSessionSetting("auth_token", auth_token);
        Settings::setSessionSetting("notestore_url", notestore_url);
        Settings::setSessionSetting("username", username);
    }

    state = tAuthState::AUTHORISED;

    return true;
}

void NoteController::getNotebookGUID()
{
    qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));
    QList<qevercloud::Notebook> notebooks = noteStore.listNotebooks();

    bool foundExistingNotebook = false;
    qevercloud::Guid notebookID;

    qInfo() << "Found" << notebooks.size() << "notebooks in user account.";
    foreach(qevercloud::Notebook item, notebooks) {
        std::string itemTitle = item.name->toUtf8().constData();
        if(itemTitle.compare("Sticky Notes") == 0)
        {
            qInfo() << "Found existing 'Sticky Notes' notebook.";
            foundExistingNotebook = true;
            notebookID = item.guid;
            break;
        }
    }

    if(!foundExistingNotebook) {
        // Creating notebook
        qevercloud::Notebook notebook;
        notebook.name = "Sticky Notes";
        notebook.defaultNotebook = false;

        qWarning() << "No existing 'Sticky Notes' notebook found. Creating a new one.";

        qevercloud::Notebook createdNotebook = noteStore.createNotebook(notebook);
        notebookID = createdNotebook.guid;
    }

    notebookGUID = notebookID;
}

void NoteController::showNotes()
{
    syncFromServer();

    std::vector<Note> sync_results = Cache::retrieveAllFromSyncTable();
    qInfo() << "Found" << sync_results.size() << "notes.";
    std::vector<Note>::iterator n = sync_results.begin();
    while(n != sync_results.end()) {
        Note note = *n;
        queueItem item = Cache::retrieveFromQueueTable(note.guid);
        noteItem size = Cache::retrieveFromNotesTable(note.guid, this->screens, this->screenWidth, this->screenHeight);

        if(item.id == NULL && item.type == NULL) {
            // Not in queue
            Note *pointer_note = new Note(note.guid, note.usn, note.title, NoteFormatter(note.content).standardiseInput());
            createNote(pointer_note, size);
        } else if(item.type == "UPDATE") {
            Note *pointer_note = new Note(note.guid, note.usn, item.note.title, NoteFormatter(item.note.content).standardiseInput());
            createNote(pointer_note, size);
        }
        n++;
    }

    // Also open new notes in queue that haven't been synced yet
    std::vector<queueItem> new_notes = Cache::retrieveNewFromQueueTable();
    std::vector<queueItem>::iterator p = new_notes.begin();

    while(p != new_notes.end()) {
        queueItem item = *p;
        Note *pointer_note = new Note(item.note.guid, item.note.usn, item.note.title, NoteFormatter(item.note.content).standardiseInput());
        createNote(pointer_note);
        p++;
    }

}

void NoteController::syncChanges()
{
    syncFromServer();

    qInfo() << "Synchronising note changes to Evernote.";

    struct GuidMap{
        QString local_guid;
        QString official_guid;

    };
    std::vector<GuidMap> changes;

    std::vector<queueItem> queue = Cache::retrieveAllFromQueueTable();
    std::vector<queueItem>::iterator n = queue.begin();

    int createdNotes = 0;
    int deletedNotes = 0;
    int updatedNotes = 0;
    while(n != queue.end()) {
        queueItem item = *n;

        if(item.type.toStdString() == "CREATE")
        {
            qevercloud::Note note;

            note.title = item.note.title;
            // If note has no title, set one...
            if(note.title == "")
                note.title = "Untitled";

            note.content = item.note.content;
            note.notebookGuid = notebookGUID;

            qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));
            qevercloud::Note created_note = noteStore.createNote(note);

            GuidMap change;
            change.local_guid = item.note.guid;
            change.official_guid = created_note.guid;
            changes.push_back(change);

            createdNotes++;
        } else if(item.type.toStdString() == "UPDATE") {
            qevercloud::Note note;
            note.title = item.note.title;
            note.content = item.note.content;

            Note storedNote = Cache::retrieveFromSyncTable(item.note.guid);

            if(item.note.usn == storedNote.usn) {
                // USN's are identical between the stored edited note, and the latest version of that note
                // stored on Evernote's servers
                note.guid = item.note.guid;
                qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));
                qevercloud::Note created_note = noteStore.updateNote(note);

                updatedNotes++;
            } else {
                qWarning() << "Conflicted change on note, pushing changes as new note.";
                note.title->append(" [CONFLICTED CHANGE ON " + QDate::currentDate().toString("dd.MM.yyyy") + "]");
                note.notebookGuid = notebookGUID;
                qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));
                qevercloud::Note created_note = noteStore.createNote(note);

                GuidMap change;
                change.local_guid = item.note.guid;
                change.official_guid = created_note.guid;
                changes.push_back(change);

                createdNotes++;
            }
        } else if(item.type.toStdString() == "DELETE") {
            qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));
            noteStore.deleteNote(item.note.guid);

            deletedNotes++;
        }

        Cache::deleteFromQueueTable(item.id);
        n++;
    }

    // Queue should now be empty
    Cache::emptyQueueTable();

    qInfo() << "Created" << createdNotes << "note(s), deleted" << deletedNotes << "note(s), updated" << updatedNotes << "note(s).";

    syncFromServer();

    foreach(NoteWidget *note, notes) {
        foreach(GuidMap change, changes) {
            if(note->note->guid == change.local_guid) {
                if(change.local_guid.toStdString().substr(0,5) != "LOCAL") {
                    Note* old_note = new Note(Cache::retrieveFromSyncTable(change.local_guid));
                    createNote(old_note);
                }
                note->note->guid = change.official_guid;
            }
        }
    }

    syncAllNoteModels();
};

void NoteController::syncFromServer()
{
    Cache::emptySyncTable();

    qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));

    qevercloud::NoteFilter noteFilter;
    noteFilter.order = qevercloud::NoteSortOrder::UPDATE_SEQUENCE_NUMBER;
    noteFilter.ascending = false;
    noteFilter.notebookGuid = this->notebookGUID;
    noteFilter.inactive = false;

    qevercloud::NotesMetadataResultSpec resultSpec;
    resultSpec.includeUpdateSequenceNum = true;

    qInfo() << "Synchronising notes from Evernote.";

    qevercloud::NotesMetadataList noteMetadata = noteStore.findNotesMetadata(noteFilter, 0, 199, resultSpec);

    foreach(qevercloud::NoteMetadata metaNote, noteMetadata.notes) {
        qevercloud::Note note = noteStore.getNote(metaNote.guid, true, false, false, false);
        Cache::insertSyncTable(Note(metaNote.guid, metaNote.updateSequenceNum, note.title, NoteFormatter(note.content).standardiseInput()));
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

void NoteController::updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height) {
    Cache::insertNotesTable(guid, this->screens, this->screenWidth, this->screenHeight, x, y, width, height);
}

void NoteController::periodicUpdate()
{
    syncChanges();
}

void NoteController::logoutPrepare()
{
    Settings::deleteSessionSettings();

    Cache::deleteDatabase();
}

void NoteController::logout()
{
    logoutPrepare();

    QMessageBox confirmationBox;
    confirmationBox.setWindowTitle("Information");
    confirmationBox.setText("This application will now exit.");
    confirmationBox.setStandardButtons(QMessageBox::Ok);

    // Only delete note and close window if user confirms 'YES'
    if(confirmationBox.exec() == QMessageBox::Ok) {
        confirmationBox.close();
        QCoreApplication::quit();

    }
}

bool NoteController::isAuthorised()
{
    return (state == tAuthState::AUTHORISED);
}
