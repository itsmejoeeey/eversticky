#ifndef CACHE_H
#define CACHE_H

#include <QSqlDatabase>

#include <QEverCloud.h>
#include "note.hpp"

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
};

class Cache
{
public:
    Cache();
    ~Cache();

    void insertSyncTable(Note note);
    void insertQueueTable(Note note);
    void deleteQueueTable(Note note);

    void emptySyncTable();
    void emptyQueueTable();

    std::vector<Note> retrieveAllFromSyncTable();
    std::vector<queueItem> retrieveAllFromQueueTable();

    std::vector<queueItem> retrieveNewFromQueueTable();

    Note retrieveFromSyncTable(qevercloud::Guid guid);
    queueItem retrieveFromQueueTable(qevercloud::Guid guid);

    void deleteFromQueueTable(int id);

    void insertNotesTable(QString guid, int screens, int res_x, int res_y, int pos_x, int pos_y, int size_x, int size_y);
    noteItem retrieveFromNotesTable(qevercloud::Guid guid, int screens, int res_x, int res_y);
    void deleteFromNotesTable(qevercloud::Guid guid);

    void deleteDatabase();
private:
    //QSqlDatabase db;
    bool dbConnected;

    QString cacheLocation;
};

#endif // CACHE_H
