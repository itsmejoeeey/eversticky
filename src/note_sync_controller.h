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


struct GuidMap{
    QString local_guid;
    QString official_guid;
};

typedef enum tAuthState {
    UNAUTHORISED,
    AUTHORISED
} tAuthState;

class NoteSyncController
{
public:
    static bool authenticate();

    static void getNotebookGUID();

    static std::vector<GuidMap> syncChanges();
    static void syncFromServer();

private:
    static qevercloud::Guid notebookGUID;
};

#endif // NOTESYNCCONTROLLER_H