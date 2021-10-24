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

#include "note_sync_controller.h"

#include <QMessageBox>

#include "cache.h"
#include "note_formatter.h"
#include "settings.h"


qevercloud::Guid NoteSyncController::notebookGUID;

bool NoteSyncController::authenticate()
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
            Settings::deleteSessionSettings();
            Cache::deleteDatabase();
        } catch(qevercloud::EDAMSystemException) {
            Settings::deleteSessionSettings();
            Cache::deleteDatabase();
        }
    }

    if(!authenticated) {
        // Test OAuth
        qevercloud::EvernoteOAuthDialog dialog(QString::fromStdString(API_KEY), QString::fromStdString(API_SECRET), QString::fromStdString(API_HOST));
        dialog.setWindowTitle("Log in to Evernote | eversticky");
        if (dialog.exec() != QDialog::Accepted) {
            // OAuth login failed
            QMessageBox::critical(0, "NotePoster", "Login failed.\n" + dialog.oauthError());

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

    getNotebookGUID();

    return true;
}

void NoteSyncController::getNotebookGUID()
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

std::vector<GuidMap> NoteSyncController::syncChanges()
{
    syncFromServer();

    qInfo() << "Synchronising note changes to Evernote.";

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

    return changes;
};

void NoteSyncController::syncFromServer()
{
    Cache::emptySyncTable();

    qevercloud::NoteStore noteStore(Settings::getSessionSetting("notestore_url"), Settings::getSessionSetting("auth_token"));

    qevercloud::NoteFilter noteFilter;
    noteFilter.order = qevercloud::NoteSortOrder::UPDATE_SEQUENCE_NUMBER;
    noteFilter.ascending = false;
    noteFilter.notebookGuid = notebookGUID;
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
