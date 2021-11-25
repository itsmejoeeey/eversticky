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

#include "note_scroll_area.h"

#include <QCommonStyle>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "ui/note_header.h"
#include "webview/note_webview.h"


NoteScrollArea::NoteScrollArea(QWidget *context) : QScrollArea(context)
{
    setObjectName("NoteScrollArea");

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setWidgetResizable(true);

    QWidget *scrollContainer = new QWidget(this);
    scrollContainer->setObjectName("NoteScrollAreaContainer");
    dockLayout = new QVBoxLayout(scrollContainer);
    dockLayout->setSpacing(0);
    dockLayout->setContentsMargins(QMargins());
    dockLayout->setAlignment(Qt::AlignTop);

    scrollContainer->setLayout(dockLayout);
    setWidget(scrollContainer);
}

void NoteScrollArea::addWidget(QWidget *widget)
{
    dockLayout->addWidget(widget);
}

void NoteScrollArea::scrollNote(int dx, int dy)
{
    const int x = horizontalScrollBar()->value() - dx;
    horizontalScrollBar()->setValue( x );

    const int y = verticalScrollBar()->value() - dy;
    verticalScrollBar()->setValue( y );
}

void NoteScrollArea::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta();

    if (!numPixels.isNull()) {
        scrollNote(numPixels.rx(), numPixels.ry());
    } else if (!numDegrees.isNull()) {
        // May need changing. I left this because I noticed numPixels and numDegrees
        // were identical on my system.
        scrollNote(numDegrees.rx(), numDegrees.ry());
    }

    event->accept();
}
