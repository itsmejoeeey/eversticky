#include "note_controller.hpp"

// Standard libs
#include <iostream>
// QT libs
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QWebEngineProfile>
// Project libs
#include "misc.hpp"
#include "note.hpp"
#include "ui/note_ui.hpp"
#include "ui/tray_ui.h"
// Other libs
#include <QEverCloudOAuth.h>


NoteController::NoteController(int screens, int screenWidth, int screenHeight) : QObject()
{
    this->screens = screens;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    TrayUI *trayIcon = new TrayUI(this);

    this->settingsFile = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).append("/user_settings.ini");
    this->state = "UNAUTHORIZED";

    if(buttonClicked()) {
        this->state = "AUTHORIZED";
        button2Clicked();
        button3Clicked();
    }

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(periodicUpdate()));
    timer->start(210000);



}

// Creating a new note to later be created in the users'
// Evernote account
NoteUI* NoteController::createNote() {
    Note* note_model = new Note();

    // Create a random local GUID (due the fact this not has not been uploaded thus far)
    // in the format 'LOCAL-XXXXXXXXXXXX-XXXXXXXXXXXX'
    QString rand = QString::fromStdString("LOCAL-" + misc::random_string(12) + "-" + misc::random_string(12));

    note_model->guid = rand;



    // Add this note to a controller that manages the UI
    NoteUI *note = new NoteUI(this, note_model, noteItem());
    notes.push_back(note);

    noteCount++;

    return note;
}

// Create a new note where a note model instance (downloaded from
// Evernote) is provided
NoteUI* NoteController::createNote(Note* note_model) {
   NoteUI *note = new NoteUI(this, note_model, noteItem());
   notes.push_back(note);

   noteCount++;

   return note;
}

NoteUI* NoteController::createNote(Note* note_model, noteItem size) {
   NoteUI *note = new NoteUI(this, note_model, size);

   notes.push_back(note);

   noteCount++;

   return note;
}


/*
 * For testing purposes only
*/

bool NoteController::buttonClicked() {
    QSettings settings(settingsFile, QSettings::NativeFormat);
    if(!(settings.value("auth_token").toString() == "" ||
         settings.value("notestore_url").toString() == "" ||
         settings.value("username").toString() == "" )) {
        //TODO Check auth token still valid
    } else {
        // Test OAuth
        qevercloud::EvernoteOAuthDialog dialog(QString::fromStdString("joejoe31b"),
                                   QString::fromStdString("7ecd76a10fe21f37"),
                                   QString::fromStdString("sandbox.evernote.com"));
        dialog.setWindowTitle("Log in to Evernote");
        if (dialog.exec() != QDialog::Accepted) {
            // OAuth login failed
            QMessageBox::critical(0, "NotePoster", "Login failed.\n" + dialog.oauthError());
            return false;
        }

        QString auth_token = dialog.oauthResult().authenticationToken;
        QString notestore_url = dialog.oauthResult().noteStoreUrl;

        qevercloud::UserStore user_store("sandbox.evernote.com", auth_token);
        qevercloud::User user = user_store.getUser();
        QString username = user.username;

        settings.setValue("auth_token", auth_token);
        settings.setValue("notestore_url", notestore_url);
        settings.setValue("username", username);
    }
    return true;
}

void NoteController::button2Clicked() {
    QSettings settings(settingsFile, QSettings::NativeFormat);
    qevercloud::NoteStore noteStore(settings.value("notestore_url").toString(), settings.value("auth_token").toString());
    QList<qevercloud::Notebook> notebooks = noteStore.listNotebooks();

    bool foundExistingNotebook = false;
    qevercloud::Guid notebookID;

    foreach(qevercloud::Notebook item, notebooks) {
        std::string itemTitle = item.name->toUtf8().constData();
        if(itemTitle.compare("Sticky Notes") == 0) {
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

        qevercloud::Notebook createdNotebook = noteStore.createNotebook(notebook);
        notebookID = createdNotebook.guid;

    }

    notebookGUID = notebookID;

}

void NoteController::button3Clicked() {
    syncFromServer();

    Cache cache;
    std::vector<Note> sync_results = cache.retrieveAllFromSyncTable();
    std::vector<Note>::iterator n = sync_results.begin();
    while(n != sync_results.end()) {
        Note note = *n;
        queueItem item = cache.retrieveFromQueueTable(note.guid);
        noteItem size = cache.retrieveFromNotesTable(note.guid, this->screens, this->screenWidth, this->screenHeight);

        if(item.id == NULL && item.type == NULL) {
            // Not in queue
            Note *pointer_note = new Note(note.guid, note.usn, note.title, note.content);
            createNote(pointer_note, size);
        } else if(item.type == "UPDATE") {
            Note *pointer_note = new Note(note.guid, note.usn, item.note.title, item.note.content);
            createNote(pointer_note, size);
        }
        n++;
    }

    // Also open new notes in queue that haven't been synced yet
    std::vector<queueItem> new_notes = cache.retrieveNewFromQueueTable();
    std::vector<queueItem>::iterator p = new_notes.begin();

    while(p != new_notes.end()) {
        queueItem item = *p;
        Note *pointer_note = new Note(item.note.guid, item.note.usn, item.note.title, item.note.content);
        createNote(pointer_note);
        p++;
    }

}

void NoteController::button4Clicked() {
    syncChanges();
}

void NoteController::syncChanges() {
    syncFromServer();

    bool errored = false;

    struct GuidMap{
        QString local_guid;
        QString official_guid;

    };
    std::vector<GuidMap> changes;

    Cache *cache = new Cache();
    std::vector<queueItem> queue = cache->retrieveAllFromQueueTable();
    std::vector<queueItem>::iterator n = queue.begin();

    while(n != queue.end()) {
        queueItem item = *n;

        if(item.type.toStdString() == "CREATE") {
            qevercloud::Note note;
            note.title = item.note.title;
            note.content = item.note.content;
            note.notebookGuid = notebookGUID;
            note.content->prepend("<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE en-note SYSTEM 'http://xml.evernote.com/pub/enml2.dtd'><en-note><div>");
            note.content->append("</div></en-note>");
            note.content->replace(QRegExp("[\n\t\r]"), "</div><div>");


            qevercloud::NoteStore noteStore("https://sandbox.evernote.com/shard/s1/notestore", "S=s1:U=95221:E=16fd4fb5ba1:C=1687d4a2e00:P=185:A=joejoe31b:V=2:H=0cab513d36220f2e8d01d172c61c94ea");
            qevercloud::Note created_note = noteStore.createNote(note);

            GuidMap change;
            change.local_guid = item.note.guid;
            change.official_guid = created_note.guid;
            changes.push_back(change);

        } else if(item.type.toStdString() == "UPDATE") {
            qevercloud::Note note;
            note.title = item.note.title;
            note.content = item.note.content;
            note.content->prepend("<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE en-note SYSTEM 'http://xml.evernote.com/pub/enml2.dtd'><en-note><div>");
            note.content->append("</div></en-note>");
            note.content->replace(QRegExp("[\n\t\r]"), "</div><div>");

            Note storedNote = cache->retrieveFromSyncTable(item.note.guid);

            if(item.note.usn == storedNote.usn) {
                // USN's are identical between the stored edited note, and the latest version of that note
                // stored on Evernote's servers
                note.guid = item.note.guid;
                qevercloud::NoteStore noteStore("https://sandbox.evernote.com/shard/s1/notestore", "S=s1:U=95221:E=16fd4fb5ba1:C=1687d4a2e00:P=185:A=joejoe31b:V=2:H=0cab513d36220f2e8d01d172c61c94ea");
                qevercloud::Note created_note = noteStore.updateNote(note);
            } else {
                note.title->append(" [CONFLICTED CHANGE ON " + QDate::currentDate().toString("dd.MM.yyyy") + "]");
                note.notebookGuid = notebookGUID;
                qevercloud::NoteStore noteStore("https://sandbox.evernote.com/shard/s1/notestore", "S=s1:U=95221:E=16fd4fb5ba1:C=1687d4a2e00:P=185:A=joejoe31b:V=2:H=0cab513d36220f2e8d01d172c61c94ea");
                qevercloud::Note created_note = noteStore.createNote(note);

                GuidMap change;
                change.local_guid = item.note.guid;
                change.official_guid = created_note.guid;
                changes.push_back(change);
            }



        } else if(item.type.toStdString() == "DELETE") {
            qevercloud::NoteStore noteStore("https://sandbox.evernote.com/shard/s1/notestore", "S=s1:U=95221:E=16fd4fb5ba1:C=1687d4a2e00:P=185:A=joejoe31b:V=2:H=0cab513d36220f2e8d01d172c61c94ea");
            noteStore.deleteNote(item.note.guid);
        }
        cache->deleteFromQueueTable(item.id);
        n++;
    }

    // Queue should already be empty, but this will clear the autoindent value if no errors occured
    if(!errored) {
        cache->emptyQueueTable();
    }

    delete cache;

    syncFromServer();

    cache = new Cache();
    foreach(NoteUI *note, notes) {
        foreach(GuidMap change, changes) {
            if(note->note->guid == change.local_guid) {
                if(change.local_guid.toStdString().substr(0,5) != "LOCAL") {
                    Note* old_note = new Note(cache->retrieveFromSyncTable(change.local_guid));
                    createNote(old_note);
                }
                note->note->guid = change.official_guid;
            }
        }
    }

    delete cache;

    syncAllNoteModels();


};

void NoteController::syncFromServer() {
    Cache cache;
    cache.emptySyncTable();

    qevercloud::NoteStore noteStore("https://sandbox.evernote.com/shard/s1/notestore", "S=s1:U=95221:E=16fd4fb5ba1:C=1687d4a2e00:P=185:A=joejoe31b:V=2:H=0cab513d36220f2e8d01d172c61c94ea");

    qevercloud::NoteFilter noteFilter;
    noteFilter.order = qevercloud::NoteSortOrder::UPDATE_SEQUENCE_NUMBER;
    noteFilter.ascending = false;
    noteFilter.notebookGuid = this->notebookGUID;
    noteFilter.inactive = false;

    qevercloud::NotesMetadataResultSpec resultSpec;
    resultSpec.includeUpdateSequenceNum = true;

    qevercloud::NotesMetadataList noteMetadata = noteStore.findNotesMetadata(noteFilter, 0, 199, resultSpec);


    foreach(qevercloud::NoteMetadata metaNote, noteMetadata.notes) {
        qevercloud::Guid guid = metaNote.guid;
        int usn = metaNote.updateSequenceNum;
        qevercloud::Note note = noteStore.getNote(guid, true, false, false, false);
        note.content->replace("<div></div>", "<br>");
        QString plainText = QTextDocumentFragment::fromHtml(note.content).toPlainText();

        cache.insertSyncTable(Note(guid, usn, note.title, plainText));
    }

}

void NoteController::syncAllNoteModels() {
    foreach(NoteUI *note, notes) {
        note->syncModel();
    }
}

QString NoteController::getUsername() {
    QSettings settings(settingsFile, QSettings::NativeFormat);
    return settings.value("username").toString();
}

void NoteController::bringAllToFront() {
    foreach(NoteUI *note, notes) {
        note->bringToForeground();
    }
}

void NoteController::updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height) {
    Cache cache;
    cache.insertNotesTable(guid, this->screens, this->screenWidth, this->screenHeight, x, y, width, height);
}

void NoteController::periodicUpdate() {
    syncChanges();
}

void NoteController::logout() {
    QFile(this->settingsFile).remove();

    Cache cache;
    cache.deleteDatabase();

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
