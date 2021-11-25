/*
 * This file is part of the EverSticky project (https://eversticky.joeeey.com).
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

#include "note_title_bar.h"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QToolButton>
#include <QDebug>


NoteTitleBar::NoteTitleBar(QWidget* parent) : QWidget(parent)
{
    setObjectName("NoteTitleBar");

    mousePressed = false;
    mouseCursorChanged = false;

    // Needed to ensure background colour is drawn
    // https://stackoverflow.com/a/23113643/3213602
    setAttribute(Qt::WidgetAttribute::WA_StyledBackground);

    setFixedHeight(40);

    QHBoxLayout *layout = new QHBoxLayout();

    QToolButton *addAction = new QToolButton(this);
    addAction->setFocusPolicy(Qt::NoFocus);
    addAction->setText("+");
    QToolButton *deleteAction = new QToolButton(this);
    deleteAction->setFocusPolicy(Qt::NoFocus);
    deleteAction->setText("x");
    connect(addAction, &QToolButton::clicked, this, &NoteTitleBar::addPressed);
    connect(deleteAction, &QToolButton::clicked, this, &NoteTitleBar::deletePressed);

    layout->addWidget(addAction);
    layout->addStretch();
    layout->addWidget(deleteAction);

    this->setLayout(layout);
}

void NoteTitleBar::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        mouseCursorChanged = false;
        mousePressPosition = event->pos();

        mousePressed = true;
    }
}

void NoteTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if(mousePressed){
        // Update mouse drag cursor only once per drag
        if(!mouseCursorChanged) {
            QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
            mouseCursorChanged = true;
        }

        // Calculate the delta between the mouse press position and the current mouse position
        QPoint mouseDelta = event->pos() - mousePressPosition;

        // Update window position
        window()->move(window()->pos() + mouseDelta);
    }
}

void NoteTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    // Return cursor to normal cursor (after being changed when window began
    // moving in NoteTitleBar)
    QApplication::restoreOverrideCursor();

    mousePressed = false;
    mouseCursorChanged = false;
}
