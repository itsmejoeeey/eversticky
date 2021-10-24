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

#ifndef NOTETITLEBAR_H
#define NOTETITLEBAR_H

#include <QMenuBar>
#include <QWidget>


class NoteTitleBar: public QWidget
{
Q_OBJECT

public:
    NoteTitleBar(QWidget* parent);

signals:
    void addPressed();
    void deletePressed();

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    bool mousePressed;
    QPoint mousePressPosition;
    bool mouseCursorChanged;
};

#endif // NOTETITLEBAR_H
