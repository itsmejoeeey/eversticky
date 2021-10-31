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

#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <QSystemTrayIcon>


class NoteController;

class TrayIcon : public QSystemTrayIcon
{
Q_OBJECT

public:
    TrayIcon(NoteController* parent);
    ~TrayIcon();

    void updateTrayMenu();

private slots:
    bool checkUpdateAvailable();

    void loginAction();
    void createAction();
    void foregroundAction();
    void syncAction();
    void settingsAction();
    void aboutAction();
    void exitAction();
    void logout();

private:
    NoteController *parent;
    QMenu* trayMenu;

    void createIconMenu(QPoint point);
};

#endif // TRAY_ICON_H
