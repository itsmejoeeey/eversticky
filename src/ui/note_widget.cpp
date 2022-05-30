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

#include "note_widget.h"

#include <iostream>

#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizeGrip>
#include <QThread>
#include <QtX11Extras/QX11Info>

#include <X11/xpm.h>

#include "note_controller.h"
#include "note_formatter.h"
#include "ui/note_header.h"
#include "ui/note_scroll_area.h"
#include "ui/note_title_bar.h"


NoteWidget::NoteWidget(NoteController *parent, Note* note, noteItem size) : parent(parent), note(note), initialNoteState(size), QMainWindow()
{
    setObjectName("NoteWindow");
    setWindowTitle("EverSticky | " + note->title);

    QIcon icon(":/icon/appicon.ico");
    this->setWindowIcon(icon);

    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // Disable window border
    setWindowFlags(Qt::FramelessWindowHint);

    // Read and apply note stylesheet
    QFile File(":style/note_stylesheet.qss");
    File.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(File.readAll());
    setStyleSheet(styleSheet);

    QFrame *noteContainer = new QFrame(this);
    noteContainer->setObjectName("NoteContainer");
    noteContainer->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    noteContainer->setLineWidth(4);

    QVBoxLayout *parentLayout = new QVBoxLayout(noteContainer);
    parentLayout->setSpacing(0);
    parentLayout->setContentsMargins(QMargins());

    // Main sticky note toolbar
    NoteTitleBar* headerBar = new NoteTitleBar(noteContainer);
    connect(headerBar, &NoteTitleBar::addPressed, this, &NoteWidget::createNote);
    connect(headerBar, &NoteTitleBar::deletePressed, this, &NoteWidget::deleteNote);
    parentLayout->setMenuBar(headerBar);

    /*
     *  Main note header and content (webview)
     */
    int notePadding = 8;
    noteScrollArea = new NoteScrollArea(noteContainer);

    noteHeader = new NoteHeader(noteScrollArea);
    noteHeader->setContentsMargins(notePadding, notePadding, notePadding, 0);
    noteScrollArea->addWidget(noteHeader);

    noteWebview = new NoteWebview(noteScrollArea);
    noteWebview->setContentsMargins(notePadding, notePadding, notePadding, 0);
    noteScrollArea->addWidget(noteWebview);

    parentLayout->addWidget(noteScrollArea);


    // These signals need to be connected AFTER title and content have been first set (after `updateUI()`)
    connect(noteHeader, &NoteHeader::textUpdated, this, &NoteWidget::titleTextUpdated);
    connect(noteWebview, &NoteWebview::ensureWebviewCaretVisible, this, &NoteWidget::scrollToWebviewCaret);
    connect(noteWebview, &NoteWebview::initialNoteLoad, this, &NoteWidget::updateUI);
    connect(noteWebview, &NoteWebview::textUpdated, this, &NoteWidget::contentTextChanged);


    // Add resize grips to the bottom corners of the note
    QHBoxLayout *gripLayout = new QHBoxLayout();
    gripLayout->addWidget(new QSizeGrip(noteContainer), 0, Qt::AlignBottom | Qt::AlignLeft);
    gripLayout->addWidget(new QSizeGrip(noteContainer), 0, Qt::AlignBottom | Qt::AlignRight);
    parentLayout->addLayout(gripLayout);

    noteContainer->setLayout(parentLayout);

    setCentralWidget(noteContainer);

    // Set window to previously cached size if possible
    setNoteSize();

    show();
    hideFromTaskbar();
};

void NoteWidget::hideFromTaskbar()
{
    // Needed to ensure the window can be found and have its state altered
    QThread::msleep(20);

    // TODO: Look into using window groups
    // http://manpages.org/x11protocolwm/3 (search for "window_group" and have a read)

    /*
     * Hide the window from the taskbar
     */
    Display *display = QX11Info::display();
    int screen = DefaultScreen(display);
    Atom type = ((Atom) 4);
    Atom property = XInternAtom(display, "_NET_WM_STATE", False);
    Atom state = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);

    WId window = this->winId();

    XChangeProperty(display, window, property, type, 32, PropModeAppend, (unsigned char*)&state, 1);

    // Tell the XServer that the state of window has been updated
    // https://mailman.linuxchix.org/pipermail/techtalk/2003-May/015402.html
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.message_type = property;
    ev.xclient.window = window;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 1;
    ev.xclient.data.l[1] = state;
    ev.xclient.data.l[2] = 0;

    XSendEvent(display, QX11Info::appRootWindow(), False,
                 SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void NoteWidget::deleteNote()
{
    QMessageBox confirmationBox;
    confirmationBox.setWindowTitle("Confirm delete");
    confirmationBox.setText("Are you sure you want to delete this note?");
    confirmationBox.setStandardButtons(QMessageBox::Yes);
    confirmationBox.addButton(QMessageBox::No);
    confirmationBox.setDefaultButton(QMessageBox::No);

    // Only delete note and close window if user confirms 'YES'
    if(confirmationBox.exec() == QMessageBox::Yes) {
        this->close();
        Cache::deleteQueueTable(*note);
        Cache::deleteFromNotesTable(note->guid);
    }
}

void NoteWidget::createNote()
{
    parent->createNote();
}

void NoteWidget::updateUI()
{
    noteHeader->setText(this->note->title);
    noteWebview->setText(NoteFormatter(this->note->content).convertFromEML());
}

void NoteWidget::scrollToWebviewCaret(int caretY)
{
    // Ensure scroll area has webview caret visible (typically after use starts typing
    // on a new line).
    noteScrollArea->ensureVisible(0, noteHeader->height() + caretY);
}

void NoteWidget::titleTextUpdated(QString newText)
{
    // If note title has changed, add it to the queue
    if(note->title != newText)
    {
        note->title = newText;
        if(note->changed == false) {
            note->changed = true;
        }

        Cache::insertQueueTable(*note);
    }
}

void NoteWidget::contentTextChanged(QString newText)
{
    // Convert back to ENML format
    newText = NoteFormatter(newText).convertToEML();

    // If note content has changed, add it to the queue
    if(note->content != newText) {
        if(note->changed == false) {
            note->changed = true;
        }
        note->content = newText;
        Cache::insertQueueTable(*note);
    }
}

void NoteWidget::syncModel()
{
    std::optional<Note> updatedNote = Cache::retrieveFromSyncTable(this->note->guid);
    std::optional<queueItem> queuedNote = Cache::retrieveFromQueueTable(this->note->guid);

    // If note doesn't exist in sync table (all fields are NULL), note has been deleted.
    if(!queuedNote && !updatedNote) {
        // Close the window
        this->close();
        return;
    } else if(queuedNote) {
        if((*queuedNote).type == "DELETE") {
            this->close();
            return;
        }
        // If note exists in the queue table (and isn't deleted), show the changed content.
        this->note = new Note((*queuedNote).note);
    } else {
        // Otherwise, show the original note.
        this->note = new Note(*updatedNote);
    }

    this->updateUI();
}

void NoteWidget::bringToForeground()
{
    // All three calls were needed to ensure window is always brought to front
    // https://stackoverflow.com/a/10808934/3213602
    this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    this->raise();
    this->activateWindow();
}

void NoteWidget::moveEvent(QMoveEvent *event)
{
    const int newX = event->pos().x();
    const int newY = event->pos().y();

    parent->updateNoteDimensions(note->guid, newX, newY, this->width(), this->height());
}

void NoteWidget::resizeEvent(QResizeEvent *event)
{
    const int newWidth = event->size().width();
    const int newHeight = event->size().height();

    parent->updateNoteDimensions(note->guid, this->x(), this->y(), newWidth, newHeight);

    event->ignore();
}

void NoteWidget::setNoteSize()
{
    if(initialNoteState.pos_x && initialNoteState.pos_y &&
       initialNoteState.size_x && initialNoteState.size_y)
    {
        this->move(initialNoteState.pos_x, initialNoteState.pos_y);
        this->resize(initialNoteState.size_x, initialNoteState.size_y);
    } else {
        resize(400,400);
    }
}

qevercloud::Guid NoteWidget::getNoteGuid()
{
    return note->guid;
}

void NoteWidget::updateNoteGuid(qevercloud::Guid newGuid)
{
    qevercloud::Guid oldGuid = note->guid;
    note->guid = newGuid;

    // Remove old note dimensions, and create entry for new note GUID.
    Cache::deleteFromNotesTable(oldGuid);
    parent->updateNoteDimensions(note->guid, this->x(), this->y(), this->width(), this->height());
}
