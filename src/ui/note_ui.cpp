#include "note_ui.hpp"

// Standard libs
#include <iostream>
// QT libs
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QSizeGrip>
#include <QThread>
#include <QtX11Extras/QX11Info>
// Project libs
#include "note_controller.hpp"
#include "ui/header_block.hpp"
// Other libs
#include <X11/xpm.h>


NoteUI::NoteUI(NoteController *parent, Note* note, noteItem size) : QWidget()
{
    this->initialized = false;

    // Keep track of the NoteController to allow us to create new
    // notes or make sure it stops tracking deleted notes
    this->parent = parent;
    this->note = note;
    setObjectName("NoteDialog");

    QFile File(":style/note_stylesheet.qss");
     File.open(QFile::ReadOnly);
     QString StyleSheet = QLatin1String(File.readAll());

     setStyleSheet(StyleSheet);

    resize(400,400);

    // Disable window border and skip taskbar
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setWindowFlags(Qt::FramelessWindowHint);

    // Main sticky note toolbar
    QToolBar *toolbar = new QToolBar();

    QAction *addAction = new QAction("+", this);
    connect(addAction, SIGNAL(triggered()), this, SLOT(addWindow()));
    toolbar->addAction(addAction);

    // Spacer widget required to right align the 'kill' button
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    toolbar->addWidget(spacerWidget);

    QAction *killAction = new QAction("x", this);
    connect(killAction, SIGNAL(triggered()), this, SLOT(killWindow()));
    toolbar->addAction(killAction);

    QVBoxLayout *dockLayout = new QVBoxLayout();
    dockLayout->setSpacing(0);
    dockLayout->setContentsMargins(QMargins());
    dockLayout->setMenuBar(toolbar);


    contentEdit = new ContentTextEdit(6, this);
    headerBlock = new HeaderBlock(contentEdit, this);


    dockLayout->addWidget(contentEdit);
    contentEdit->attachSameScroll(headerBlock);

    QHBoxLayout *gripLayout = new QHBoxLayout();

    QSizeGrip *sizeGripLeft = new QSizeGrip(this);
    gripLayout->addWidget(sizeGripLeft, 0, Qt::AlignBottom | Qt::AlignLeft);
    QSizeGrip *sizeGripRight = new QSizeGrip(this);
    gripLayout->addWidget(sizeGripRight, 0, Qt::AlignBottom | Qt::AlignRight);
    dockLayout->addLayout(gripLayout);

    setLayout(dockLayout);

    updateUI();

    // Set window title and icon
    this->setWindowTitle("EverSticky | " + note->title);
    QIcon icon(":/icon/icon.ico");
    this->setWindowIcon(icon);

    show();

    // Needed to ensure the window can be found and have its state altered
    QThread::msleep(20);


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

    if(size.pos_x && size.pos_y && size.size_x && size.size_y) {
        this->setNoteSize(size);
    }

    this->initialized = true;

};

void NoteUI::killWindow() {
    QMessageBox confirmationBox;
    confirmationBox.setWindowTitle("Confirm delete");
    confirmationBox.setText("Are you sure you want to delete this note?");
    confirmationBox.setStandardButtons(QMessageBox::Yes);
    confirmationBox.addButton(QMessageBox::No);
    confirmationBox.setDefaultButton(QMessageBox::No);

    // Only delete note and close window if user confirms 'YES'
    if(confirmationBox.exec() == QMessageBox::Yes) {
        this->close();
        Cache cache;
        cache.deleteQueueTable(*note);
        cache.deleteFromNotesTable(note->guid);
    }

}



void NoteUI::addWindow() {
    parent->createNote();
}

void NoteUI::updateUI() {
    headerBlock->setText(this->note->title);
    contentEdit->setText(this->note->content);

}

void NoteUI::titleTextChanged() {
        if(headerBlock->getText() != note->title) {
            if(note->changed == false) {
                note->changed = true;
            }
            note->title = headerBlock->getText();
            Cache cache;
            cache.insertQueueTable(*note);
        }

}

void NoteUI::contentTextChanged(QString newText) {
        if(newText != note->content) {
            if(note->changed == false) {
                note->changed = true;
            }
            note->content = newText;
            Cache cache;
            cache.insertQueueTable(*note);
        }

}

qevercloud::Guid NoteUI::getGUID() {
    return note->guid;
}

void NoteUI::updateModel(Note *note) {
    this->note = note;
    this->updateUI();
}

void NoteUI::syncModel() {
    headerBlock->clearText();
    contentEdit->clearText();
    Cache cache;
    Note *updatedNote = new Note(cache.retrieveFromSyncTable(this->note->guid));
    this->note = updatedNote;
    this->updateUI();
}

void NoteUI::bringToForeground() {
    this->activateWindow();
}

void NoteUI::moveEvent(QMoveEvent *event) {
    // Ensured initialized to prevent null GUID
    if(initialized) {
        int newX = event->pos().x();
        int newY = event->pos().y();

        parent->updateNoteDimensions(this->getGUID(), newX, newY, this->width(), this->height());
    }
}

void NoteUI::resizeEvent(QResizeEvent *event) {
    // Ensured initialized to prevent null GUID
    if(initialized) {
        int newWidth = event->size().width();
        int newHeight = event->size().height();

        parent->updateNoteDimensions(this->getGUID(), this->x(), this->y(), newWidth, newHeight);
    }
}

void NoteUI::setNoteSize(noteItem size) {
    this->move(size.pos_x, size.pos_y);
    this->resize(size.size_x, size.size_y);
}


