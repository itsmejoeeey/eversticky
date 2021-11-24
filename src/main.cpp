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

#include <iostream>

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

#include "note_controller.h"


int main(int argc, char **argv)
{
    QApplication app (argc, argv);
    app.setApplicationName("eversticky");
    app.setQuitOnLastWindowClosed(false);

    // Show timestamp in logging output
    qSetMessagePattern("[%{time}] %{message}");

    const int numScreens = app.screens().length();
    const QRect screenSize = app.primaryScreen()->virtualGeometry();
    // *.* Where the magic happens *.*
    new NoteController(numScreens, screenSize.width(), screenSize.height());

    return app.exec();
}


