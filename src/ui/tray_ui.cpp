#include "tray_ui.hpp"

// QT Libs
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>


void trayEvent();

TrayUI::TrayUI(NoteController* parent) : QSystemTrayIcon()
{
    this->parent = parent;

    QIcon icon(":/icon/icon.ico");
    this->setIcon(icon);


    this->show();
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayEvent(QSystemTrayIcon::ActivationReason)));
}

void TrayUI::trayEvent(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
        case QSystemTrayIcon::Context:
            createIconMenu(QCursor::pos());
            break;
        case QSystemTrayIcon::Trigger:
            foregroundAction();
            break;
        default:
            break;
    }
}

void TrayUI::createIconMenu(QPoint point) {
    QMenu menu;
    menu.setContextMenuPolicy(Qt::CustomContextMenu);

    if(parent->state == "AUTHORIZED") {
        QAction *login_action = new QAction();
        login_action->setText("Logged in as " + parent->getUsername());
        login_action->setEnabled(false);
        menu.addAction(login_action);

        menu.addAction("↪ Logout", this, SLOT(logout()));
    } else {
        QAction *login_action = new QAction();
        login_action->setText("Not logged in!");
        login_action->setEnabled(false);
        menu.addAction(login_action);
    }

    menu.addSeparator();

    if(parent->state == "AUTHORIZED") {
        menu.addAction("Create note", this, SLOT(createAction()));
        menu.addAction("Force sync now", this, SLOT(syncAction()));
        menu.addAction("Bring notes to foreground", this, SLOT(foregroundAction()));
    } else {
        menu.addAction("Login", this, SLOT(loginAction()));
    }

    menu.addSeparator();

    menu.addAction("Exit", this, SLOT(exitAction()));
    menu.exec(point);
}

void TrayUI::loginAction() {
    if(parent->buttonClicked()) {
        parent->state = "AUTHORIZED";
        parent->button2Clicked();
        parent->button3Clicked();
    }
}

void TrayUI::createAction() {
    parent->createNote();
}

void TrayUI::syncAction() {
    parent->button4Clicked();
}

void TrayUI::foregroundAction() {
    parent->bringAllToFront();
}

void TrayUI::exitAction() {
    // Not recommended, cheap solution
    QCoreApplication::quit();
}

void TrayUI::logout() {
    parent->logout();
}
