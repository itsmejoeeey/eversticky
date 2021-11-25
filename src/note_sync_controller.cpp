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

#include <QIcon>
#include <QMessageBox>

#include "cache.h"
#include "note_formatter.h"
#include "settings.h"


/*
 *  Static functions
 */

tAuthState NoteSyncController::login()
{
    // Open OAuth dialog
    qevercloud::EvernoteOAuthDialog dialog(QString::fromStdString(API_KEY), QString::fromStdString(API_SECRET), QString::fromStdString(API_HOST));
    QIcon icon(":/icon/appicon.ico");
    dialog.setWindowIcon(icon);
    dialog.setWindowTitle("EverSticky | Log in to Evernote");

    if (dialog.exec() != QDialog::Accepted) {
        // OAuth login failed
        QMessageBox failedBox;
        failedBox.setIcon(QMessageBox::Critical);
        failedBox.setWindowIcon(icon);
        failedBox.setWindowTitle("Critical");
        failedBox.setText("Login failed.\n" + dialog.oauthError());
        failedBox.exec();

        return tAuthState::UNAUTHORISED;
    }

    QString auth_token = dialog.oauthResult().authenticationToken;
    QString notestore_url = dialog.oauthResult().noteStoreUrl;

    // Get username associated with login
    qevercloud::UserStore user_store(QString::fromStdString(API_HOST), auth_token);
    qevercloud::User user = user_store.getUser();
    QString username = user.username;

    // If the authenticated username differs from the previous username (or there is no previous username),
    // delete all the data from the previous user/runtime.
    if(Settings::getSessionSetting("username") != username)
    {
        Settings::deleteAllSessionSettings();
        Cache::deleteDatabase();

        Settings::setSessionSetting("username", username);
    }
    Settings::setSessionSetting("auth_token", auth_token);
    Settings::setSessionSetting("notestore_url", notestore_url);
    Settings::setSessionSetting("notebook_guid", createOrGetNotebookGUID(auth_token, notestore_url));

    return tAuthState::AUTHORISED;
}

// Check if the user already has a sticky notes notebook. Create one if necessary.
qevercloud::Guid NoteSyncController::createOrGetNotebookGUID(QString authToken, QString notestoreUrl)
{
    qevercloud::NoteStore noteStore(notestoreUrl, authToken);
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

    return notebookID;
}


/*
 *  Class functions
 */

NoteSyncController::NoteSyncController(QString authToken, qevercloud::Guid notebookGuid, QString notestoreUrl) : QObject(), notebookGuid(notebookGuid)
{
    noteStore = new qevercloud::NoteStore(notestoreUrl, authToken);
}

std::vector<GuidMap> NoteSyncController::syncChanges()
{
    bool syncSuccess = syncFromServer();

    // Return immediately if sync failed or there are no changes to sync.
    if(!syncSuccess || Cache::countQueueTableRows() == 0) {
        return {};
    }

    qInfo() << "Synchronising note changes to Evernote.";

    std::vector<GuidMap> changes;
    std::vector<queueItem> queue = Cache::retrieveAllFromQueueTable();
    std::vector<queueItem>::iterator n = queue.begin();

    int createdNotes = 0;
    int deletedNotes = 0;
    int updatedNotes = 0;
    try {
        while(n != queue.end())
        {
            queueItem item = *n;

            // *- CREATE note -*
            if(item.type == "CREATE") {
                qevercloud::Guid createdNoteGuid = syncCreateNote(item.note);
                changes.push_back(GuidMap({
                    .local_guid = item.note.guid,
                    .official_guid = createdNoteGuid
                }));
                createdNotes++;
            }
            // *- UPDATE note -*
            else if(item.type == "UPDATE") {
                Note storedNote = Cache::retrieveFromSyncTable(item.note.guid);

                // Check if the remote note has been changed more recently.
                // If the USNs match between the the remote note and the changed note, the
                // change is not conflicting.
                if(item.note.usn == storedNote.usn) {
                    syncUpdateNote(item.note);
                    updatedNotes++;
                }
                // USNs do not match. Changes are conflicting.
                // To avoid data loss, create a new note with the changes.
                else {
                    qWarning() << "Conflicted change on note, pushing changes as new note.";
                    item.note.title.append(" [CONFLICTED CHANGE ON " + QDate::currentDate().toString("dd.MM.yyyy") + "]");

                    qevercloud::Guid createdNoteGuid = syncCreateNote(item.note);
                    createdNotes++;
                }
            }
            // *- DELETE note -*
            else if(item.type == "DELETE") {
                syncDeleteNote(item.note);
                deletedNotes++;
            }

            Cache::deleteFromQueueTable(item.id);
            n++;
        }
    }
    // Provide more detail when an exception occurs that the user may be able to resolve
    // The handling of errors will be changed when the QEverCloud is updated to the latest version.
    catch(qevercloud::EDAMUserException e) {
        qCritical() << "An exception occured while syncronising changes with Evernote servers. Some changes may not have synchronised.";

        if(e.errorCode == qevercloud::EDAMErrorCode::AUTH_EXPIRED) {
            qCritical() << "Exception reason:" << "\"Authentication token expired. You will need to login again.\"";
            emit authInvalid();
        } else if(e.errorCode == qevercloud::EDAMErrorCode::ENML_VALIDATION) {
            qCritical() << "Exception reason:" << "\"Content of a submitted note was malformed.\"";
        } else {
            qCritical() << "Exception error code:" << e.errorCode;
        }

        return changes;
    }
    // Catch all Evernote exceptions
    catch(qevercloud::EverCloudException e) {
        qCritical() << "An exception occured while syncronising changes with Evernote servers. Some changes may not have synchronised.";
        qCritical() << "Exception message:" << e.exceptionData()->errorMessage;

        // Stop prematurely. Hopefully next time will succeed.
        return changes;
    }

    // Queue should now be empty
    Cache::emptyQueueTable();

    qInfo() << "Created" << createdNotes << "note(s), deleted" << deletedNotes << "note(s), updated" << updatedNotes << "note(s).";

    // Need to sync again to ensure sync table reflects created and deleted notes.
    syncFromServer();

    return changes;
};

// Return true if sync successful
bool NoteSyncController::syncFromServer()
{
    Cache::emptySyncTable();

    qevercloud::NoteFilter noteFilter;
    noteFilter.order = qevercloud::NoteSortOrder::UPDATE_SEQUENCE_NUMBER;
    noteFilter.ascending = false;
    noteFilter.notebookGuid = notebookGuid;
    noteFilter.inactive = false;

    qevercloud::NotesMetadataResultSpec resultSpec;
    resultSpec.includeUpdateSequenceNum = true;

    qInfo() << "Synchronising notes from Evernote.";

    qevercloud::NotesMetadataList noteMetadata;
    try {
        noteMetadata = noteStore->findNotesMetadata(noteFilter, 0, 199, resultSpec);
    }
    // Provide more detail when an exception occurs that the user may be able to resolve
    catch(qevercloud::EDAMUserException e) {
        qCritical() << "An exception occured while syncronising changes with Evernote servers. Some changes may not have synchronised.";

        // Explicitly catch exception caused by an expired session
        if(e.errorCode == qevercloud::EDAMErrorCode::AUTH_EXPIRED) {
            qCritical() << "Exception reason:" << "\"Authentication token expired. You will need to login again.\"";
            emit authInvalid();
        } else {
            qCritical() << "Exception error code:" << e.errorCode;
        }
        return false;
    }
    // Exit prematurely on any Evernote exception
    catch (qevercloud::EverCloudException e) {
        qCritical() << "An exception occured while syncronising with Evernote servers.";
        qCritical() << "Exception message:" << e.exceptionData()->errorMessage;

        return false;
    }

    foreach(qevercloud::NoteMetadata metaNote, noteMetadata.notes) {
        qevercloud::Note note = noteStore->getNote(metaNote.guid, true, false, false, false);
        Cache::insertSyncTable(Note(metaNote.guid, metaNote.updateSequenceNum, note.title, NoteFormatter(note.content).standardiseInput()));
    }

    qInfo() << "Found" << noteMetadata.notes.size() << "notes.";

    return true;
}

qevercloud::Guid NoteSyncController::syncCreateNote(Note note)
{
    qevercloud::Note cleanedNote;

    // If note has no title, set one...
    if(note.title == "") {
        cleanedNote.title = "Untitled";
    } else {
        cleanedNote.title = note.title;
    }

    cleanedNote.content = note.content;
    cleanedNote.notebookGuid = Settings::getSessionSetting("notebook_guid");

    qevercloud::Note created_note = noteStore->createNote(cleanedNote);

    // Return the guid of the newly created note.
    return created_note.guid;
}

void NoteSyncController::syncDeleteNote(Note note)
{
    noteStore->deleteNote(note.guid);
}

void NoteSyncController::syncUpdateNote(Note note)
{
    qevercloud::Note cleanedNote;

    cleanedNote.title = note.title;
    cleanedNote.content = note.content;
    cleanedNote.guid = note.guid;

    qevercloud::Note created_note = noteStore->updateNote(cleanedNote);
}
