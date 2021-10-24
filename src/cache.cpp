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

#include "cache.h"

#include <iostream>

#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>


QSqlDatabase Cache::openDatabase()
{
    // Return database connection immediately if it already exists
    if(QSqlDatabase::database("CacheDatabase").isOpen()) {
        return QSqlDatabase::database("CacheDatabase");
    }

    // Use SQLite format for db
    const QString DRIVER("QSQLITE");

    if(QSqlDatabase::isDriverAvailable(DRIVER)) {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER, "CacheDatabase");
        QString databasePath = getDatabasePath();
        db.setDatabaseName(databasePath);
        db.open();

        // Create tables if database doesn't exist
        QSqlQuery("CREATE TABLE sync (guid TEXT PRIMARY KEY, usn INTEGER, title TEXT, content TEXT)", db);
        QSqlQuery("CREATE TABLE queue (id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT, guid TEXT, usn INTEGER, title TEXT, content TEXT, cleared INTEGER)", db);
        QSqlQuery("CREATE TABLE notes (id INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT, screens INTEGER, res_x INTEGER, res_y INTEGER,"
                  "pos_x INTEGER, pos_y INTEGER, size_x INTEGER, size_y INTEGER, scroll_x INTEGER, scroll_y INTEGER, colour INTEGER, zindex INTEGER)", db);

        return db;
    }
}

QString Cache::getDatabasePath()
{
    // Get os-specific app-data directory for db storage
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).append("/cache.db");
}

void Cache::closeDatabase() {
    QSqlDatabase db = QSqlDatabase::database("CacheDatabase");

    if(db.isOpen())
        db.close();

    if(QSqlDatabase::database("CacheDatabase").isValid())
        QSqlDatabase::removeDatabase("CacheDatabase");
}

void Cache::insertSyncTable(Note note)
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("INSERT INTO sync (guid, usn, title, content) "
                  "VALUES (:guid, :usn, :title, :content)");

    query.bindValue(":guid", note.guid);
    query.bindValue(":usn", note.usn);
    query.bindValue(":title", note.title);
    query.bindValue(":content", note.content);
    query.exec();
}

Note Cache::retrieveFromSyncTable(qevercloud::Guid guid)
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT guid, usn, title, content FROM sync WHERE guid == :guid");
    query.bindValue(":guid", guid.toUtf8().constData());
    query.exec();

    query.next();

    return Note(query.value(0).toString(), query.value(1).toInt(), query.value(2).toString(), query.value(3).toString());
}

queueItem Cache::retrieveFromQueueTable(qevercloud::Guid guid)
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT id, type, guid, usn, title, content FROM queue WHERE guid == :guid");
    query.bindValue(":guid", guid.toUtf8().constData());
    query.exec();

    if(query.next()) {
        queueItem item;
        item.id = query.value(0).toInt();
        item.type = query.value(1).toString();
        item.note = Note(query.value(2).toString(), query.value(3).toInt(), query.value(4).toString(), query.value(5).toString());
        return item;
    } else {
        queueItem item;
        item.id = NULL;
        item.type = "";
        item.note = Note(NULL, NULL, NULL, NULL);
        return item;
    }
}

std::vector<queueItem> Cache::retrieveNewFromQueueTable()
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT id, type, guid, usn, title, content FROM queue WHERE type == :type");
    query.bindValue(":type", "CREATE");
    query.exec();

    std::vector<queueItem> results;
    while(query.next()) {
        queueItem item;
        item.id = query.value(0).toInt();
        item.type = query.value(1).toString();
        item.note = Note(query.value(2).toString(), query.value(3).toInt(), query.value(4).toString(), query.value(5).toString());
        results.push_back(item);
    }

    return results;
}


std::vector<Note> Cache::retrieveAllFromSyncTable()
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT guid, usn, title, content FROM sync");
    query.exec();

    std::vector<Note> results;
    while(query.next()) {
        results.push_back(Note(query.value(0).toString(), query.value(1).toInt(), query.value(2).toString(), query.value(3).toString()));
    }

    return results;
}

void Cache::insertQueueTable(Note note)
{
    QSqlDatabase db = openDatabase();

    QString type;
    QSqlQuery query1(db);
    if(note.new_note) {
        type = "CREATE";
    } else {
        type = "UPDATE";
    }

    query1.prepare("SELECT id FROM queue WHERE guid == :guid AND type == :type");
    query1.bindValue(":guid", note.guid);
    query1.bindValue(":type", type);
    query1.exec();

    if(query1.next()) {

        int rowId = query1.value(0).toInt();

        QSqlQuery query2(db);
        query2.prepare("UPDATE queue SET type=:type, guid=:guid, usn=:usn, title=:title, content=:content, cleared=:cleared WHERE id=:id");
        query2.bindValue(":id", rowId);
        query2.bindValue(":type", type);
        query2.bindValue(":guid", note.guid);
        query2.bindValue(":usn", note.usn);
        query2.bindValue(":title", note.title);
        query2.bindValue(":content", note.content);
        query2.bindValue(":cleared", 0);
        query2.exec();


    } else {
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO queue (type, guid, usn, title, content, cleared) "
                      "VALUES (:type, :guid, :usn, :title, :content, :cleared)");
        query2.bindValue(":type", type);
        query2.bindValue(":guid", note.guid);
        query2.bindValue(":usn", note.usn);
        query2.bindValue(":title", note.title);
        query2.bindValue(":content", note.content);
        query2.bindValue(":cleared", 0);
        query2.exec();
    }




    //db.close();
}

std::vector<queueItem> Cache::retrieveAllFromQueueTable()
{
    QSqlDatabase db = openDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT id, type, guid, usn, title, content FROM queue");
    query.exec();

    std::vector<queueItem> results;
    while(query.next()) {
        queueItem item;
        item.id = query.value(0).toInt();
        item.type = query.value(1).toString();
        item.note = Note(query.value(2).toString(), query.value(3).toInt(), query.value(4).toString(), query.value(5).toString());
        results.push_back(item);
    }

    return results;
}

void Cache::deleteQueueTable(Note note)
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query1(db);
    query1.prepare("DELETE FROM queue WHERE guid=:guid");
    query1.bindValue(":guid", note.guid.toUtf8().constData());
    query1.exec();

    // Only insert DELETE command into queue if the note has previously
    // been synced with Evernote.
    if(!note.guid.startsWith("LOCAL-"))
    {
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO queue (type, guid, title, content, cleared) "
                       "VALUES (:type, :guid, NULL, NULL, :cleared)");
        query2.bindValue(":type", "DELETE");
        query2.bindValue(":guid", note.guid.toUtf8().constData());
        query2.bindValue(":cleared", 0);
        query2.exec();
    }
}

void Cache::deleteFromQueueTable(int id)
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query(db);
    query.prepare("DELETE FROM queue WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();
}

void Cache::emptySyncTable()
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query(db);
    query.prepare("DELETE FROM sync");
    query.exec();
}

void Cache::emptyQueueTable()
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query1(db);
    query1.prepare("DELETE FROM queue");
    query1.exec();

    QSqlQuery query2(db);
    query2.prepare("DELETE FROM sqlite_sequence WHERE name = 'queue'");
    query2.exec();
}

void Cache::insertNotesTable(qevercloud::Guid guid, int screens, int res_x, int res_y, int pos_x, int pos_y, int size_x, int size_y)
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query1(db);
    query1.prepare("SELECT id FROM notes WHERE guid == :guid AND screens == :screens AND res_x == :res_x AND res_y == :res_y");
    query1.bindValue(":guid", guid.toUtf8().constData());
    query1.bindValue(":screens", screens);
    query1.bindValue(":res_x", res_x);
    query1.bindValue(":res_y", res_y);
    query1.exec();


    if(query1.next()) {
        int rowId = query1.value(0).toInt();

        QSqlQuery query2(db);
        query2.prepare("UPDATE notes SET guid = :guid, screens = :screens, res_x = :res_x, res_y = :res_y, pos_x = :pos_x, pos_y = :pos_y,"
                       "size_x = :size_x, size_y = :size_y, scroll_x = :scroll_x, scroll_y = :scroll_y WHERE id=:id");
        query2.bindValue(":guid", guid.toUtf8().constData());
        query2.bindValue(":screens", screens);
        query2.bindValue(":res_x", res_x);
        query2.bindValue(":res_y", res_y);
        query2.bindValue(":pos_x", pos_x);
        query2.bindValue(":pos_y", pos_y);
        query2.bindValue(":size_x", size_x);
        query2.bindValue(":size_y", size_y);
        query2.bindValue(":id", rowId);
        query2.exec();

    } else {
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO notes (guid, screens, res_x, res_y, pos_x, pos_y, size_x, size_y, scroll_x, scroll_y) "
                      "VALUES (:guid, :screens, :res_x, :res_y, :pos_x, :pos_y, :size_x, :size_y, :scroll_x, :scroll_y)");
        query2.bindValue(":guid", guid.toUtf8().constData());
        query2.bindValue(":screens", screens);
        query2.bindValue(":res_x", res_x);
        query2.bindValue(":res_y", res_y);
        query2.bindValue(":pos_x", pos_x);
        query2.bindValue(":pos_y", pos_y);
        query2.bindValue(":size_x", size_x);
        query2.bindValue(":size_y", size_y);
        query2.exec();
    }
}

noteItem Cache::retrieveFromNotesTable(qevercloud::Guid guid, int screens, int res_x, int res_y)
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query(db);
    query.prepare("SELECT pos_x, pos_y, size_x, size_y FROM notes WHERE guid == :guid AND screens == :screens AND res_x == :res_x AND res_y == :res_y");
    query.bindValue(":guid", guid.toUtf8().constData());
    query.bindValue(":screens", screens);
    query.bindValue(":res_x", res_x);
    query.bindValue(":res_y", res_y);
    query.exec();

    noteItem item = noteItem();
    if(query.next()) {
        item.pos_x = query.value(0).toInt();
        item.pos_y = query.value(1).toInt();
        item.size_x = query.value(2).toInt();
        item.size_y = query.value(3).toInt();
    }
    return item;
}

void Cache::deleteFromNotesTable(qevercloud::Guid guid)
{
    QSqlDatabase db = openDatabase();

    QSqlQuery query(db);
    query.prepare("DELETE FROM notes WHERE guid=:guid");
    query.bindValue(":guid", guid.toUtf8().constData());
    query.exec();
}

void Cache::deleteDatabase()
{
    // Return immediately if the database connection already exists
    if(QSqlDatabase::database("CacheDatabase").isValid())
    {
        QSqlDatabase::removeDatabase("CacheDatabase");

        QString databasePath = getDatabasePath();
        QFile(databasePath).remove();
    };

}
