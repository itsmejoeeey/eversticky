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

#ifndef NOTESCROLLAREA_H
#define NOTESCROLLAREA_H

#include <QScrollArea>
#include <QVBoxLayout>


class NoteWebview;
class NoteHeader;
class NoteWidget;

class NoteScrollArea : public QScrollArea
{
Q_OBJECT

public:
    NoteScrollArea(QWidget *context);

    void addWidget(QWidget *widget);

signals:
    void noteScrollChanged(int x, int y);

private:
    QVBoxLayout *dockLayout;
    void scrollNote(int dx, int dy);
    void wheelEvent(QWheelEvent *event);
};

#endif // NOTESCROLLAREA_H
