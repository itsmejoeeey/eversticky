/*
 * This file is part of the eversticky project (https://github.com/itsmejoeeey/eversticky).
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

#ifndef CACHE_H
#define CACHE_H

#include <QStandardPaths>
#include <QSqlDatabase>

#include <QEverCloud.h>

#include "note.h"


struct queueItem {
    int id;
    QString type;
    Note note;
};

struct noteItem {
    int pos_x;
    int pos_y;
    int size_x;
    int size_y;
    int scroll_x;
    int scroll_y;
};

class Cache
{
public:
    static QSqlDatabase openDatabase();
    static void closeDatabase();
    static void deleteDatabase();
    static QString getDatabasePath();

    static void emptySyncTable();
    static void insertSyncTable(Note note);
    static Note retrieveFromSyncTable(qevercloud::Guid guid);
    static std::vector<Note> retrieveAllFromSyncTable();

    static int countQueueTableRows();
    static void deleteQueueTable(Note note);
    static void deleteFromQueueTable(int id);
    static void emptyQueueTable();
    static void insertQueueTable(Note note);
    static queueItem retrieveFromQueueTable(qevercloud::Guid guid);
    static std::vector<queueItem> retrieveAllFromQueueTable();
    static std::vector<queueItem> retrieveNewFromQueueTable();

    static void deleteFromNotesTable(qevercloud::Guid guid);
    static void insertNotesTable(QString guid, int screens, int res_x, int res_y, int pos_x, int pos_y, int size_x, int size_y);
    static noteItem retrieveFromNotesTable(qevercloud::Guid guid, int screens, int res_x, int res_y);
};

#endif // CACHE_H
