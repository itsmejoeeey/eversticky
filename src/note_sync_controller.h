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

#ifndef NOTESYNCCONTROLLER_H
#define NOTESYNCCONTROLLER_H

#include <QEverCloudOAuth.h>

#include <note.h>


struct GuidMap{
    qevercloud::Guid local_guid;
    qevercloud::Guid official_guid;
};

typedef enum tAuthState {
    UNAUTHORISED,
    AUTHORISED
} tAuthState;

class NoteSyncController : public QObject
{
Q_OBJECT

public:
    NoteSyncController(QString authToken, qevercloud::Guid notebookGuid, QString notestoreUrl);

    static tAuthState login();

    std::vector<GuidMap> syncChanges();
    bool syncFromServer();

signals:
    void authInvalid();

private:
    qevercloud::Guid notebookGuid;
    qevercloud::NoteStore* noteStore;

    static qevercloud::Guid createOrGetNotebookGUID(QString authToken, QString notestoreUrl);

    qevercloud::Guid syncCreateNote(Note note);
    void syncDeleteNote(Note note);
    void syncUpdateNote(Note note);
};

#endif // NOTESYNCCONTROLLER_H
