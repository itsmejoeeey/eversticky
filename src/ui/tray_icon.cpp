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

#include "tray_icon.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>

#include <settings.h>
#include <ui/about_widget.h>
#include <ui/settings_widget.h>


void trayEvent();

TrayIcon::TrayIcon(NoteController* parent) : parent(parent), QSystemTrayIcon()
{
    QIcon icon;
    if(Settings::getUserSetting("dark_status_icon").toBool())
        icon = QIcon(":/icon/trayicon_black.ico");
    else
        icon = QIcon(":/icon/trayicon_white.ico");
    setIcon(icon);

    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::trayEvent);

    show();

    if(checkUpdateAvailable())
    {
        showMessage("Update Available", "A new version of eversticky has been released - click to download.", icon, 5000);

        // Known problems with KDE Plasma
        // See https://bugreports.qt.io/browse/QTBUG-87329
        connect(this, &QSystemTrayIcon::messageClicked, this, [](){
            QDesktopServices::openUrl(QUrl("https://github.com/itsmejoeeey/eversticky/releases"));
        });
    }
}

void TrayIcon::trayEvent(QSystemTrayIcon::ActivationReason reason)
{
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

bool TrayIcon::checkUpdateAvailable()
{
    QString url = "https://api.github.com/repos/itsmejoeeey/eversticky/releases/latest";
    QNetworkAccessManager manager;
    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));

    QEventLoop event;
    connect(response, SIGNAL(finished()), &event, SLOT(quit()));
    event.exec();

    QJsonDocument doc = QJsonDocument::fromJson(response->readAll());
    response->deleteLater();

    QString current_tag = QString::fromStdString(APP_VERSION);
    QString github_tag = doc.object().toVariantMap()["tag_name"].toString();

    if(github_tag.size() != 0 && github_tag > current_tag) {
        return true;
    }

    return false;
}

void TrayIcon::createIconMenu(QPoint point)
{
    QMenu menu;
    menu.setContextMenuPolicy(Qt::CustomContextMenu);

    if(parent->isAuthorised()) {
        QAction *login_action = new QAction();
        login_action->setText("Logged in as " + Settings::getSessionSetting("username"));
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

    if(parent->isAuthorised()) {
        menu.addAction("Create note", this, SLOT(createAction()));
        menu.addAction("Force sync now", this, SLOT(syncAction()));
        menu.addAction("Bring notes to foreground", this, SLOT(foregroundAction()));
    } else {
        menu.addAction("Login", this, SLOT(loginAction()));
    }

    menu.addSeparator();

    menu.addAction("Settings", this, SLOT(settingsAction()));
    menu.addAction("About", this, SLOT(aboutAction()));
    menu.addAction("Exit", this, SLOT(exitAction()));
    menu.exec(point);
}

void TrayIcon::loginAction()
{
    parent->login();
}

void TrayIcon::createAction()
{
    parent->createNote();
}

void TrayIcon::syncAction()
{
    parent->periodicUpdate();
}

void TrayIcon::foregroundAction()
{
    parent->bringAllToFront();
}

void TrayIcon::settingsAction()
{
    new SettingsWidget();
}

void TrayIcon::aboutAction()
{
    new AboutWidget();
}

void TrayIcon::exitAction()
{
    // Not recommended, cheap solution
    QCoreApplication::quit();
}

void TrayIcon::logout()
{
    parent->logout();
}
