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

#include "tray_icon.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>

#include <note_controller.h>
#include <settings.h>
#include <ui/about_widget.h>
#include <ui/settings_widget.h>


TrayIcon::TrayIcon(NoteController* parent) : parent(parent), QSystemTrayIcon()
{
    QIcon icon;
    if(Settings::getUserSetting("dark_status_icon").toBool())
        icon = QIcon(":/icon/trayicon_black.ico");
    else
        icon = QIcon(":/icon/trayicon_white.ico");
    setIcon(icon);

    trayMenu = new QMenu();
    trayMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    updateTrayMenu();
    setContextMenu(trayMenu);

    // Bring notes to foreground on tray icon click.
    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::foregroundAction);

    show();

    if(Settings::getUserSetting("check_for_updates").toBool() && checkUpdateAvailable())
    {
        // Delay showing the notification until the application should be fully loaded (10s later).
        QTimer::singleShot(10000, this, [this](){
            showUpdateAvailableNotification();
        });
    }
}

TrayIcon::~TrayIcon()
{
    delete trayMenu;
}

// Returns true if a newer version of EverSticky is available.
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

    QString current_tag = "v" + QString::fromStdString(APP_VERSION);
    QString github_tag = doc.object().toVariantMap()["tag_name"].toString();

    if(github_tag.size() != 0 && github_tag > current_tag) {
        return true;
    }

    return false;
}

void TrayIcon::showUpdateAvailableNotification()
{
    showMessage("Update Available", "A new version of eversticky has been released - click to download.", icon(), 5000);

    // TODO: Known problems with KDE Plasma - needs more investigation.
    // See https://bugreports.qt.io/browse/QTBUG-87329
    connect(this, &QSystemTrayIcon::messageClicked, this, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/itsmejoeeey/eversticky/releases"));
    });
}

void TrayIcon::updateTrayMenu()
{
    trayMenu->clear();

    if(parent->isAuthorised()) {
        QAction *login_action = new QAction();
        login_action->setText("Logged in as " + Settings::getSessionSetting("username"));
        login_action->setEnabled(false);
        trayMenu->addAction(login_action);

        trayMenu->addAction("↪ Logout", this, SLOT(logout()));
    } else {
        QAction *login_action = new QAction();
        login_action->setText("Not logged in!");
        login_action->setEnabled(false);
        trayMenu->addAction(login_action);
    }

    trayMenu->addSeparator();

    if(parent->isAuthorised()) {
        trayMenu->addAction("Create note", this, SLOT(createAction()));
        trayMenu->addAction("Force sync now", this, SLOT(syncAction()));
        trayMenu->addAction("Bring notes to foreground", this, SLOT(foregroundAction()));
    } else {
        trayMenu->addAction("Login", this, SLOT(loginAction()));
    }

    trayMenu->addSeparator();

    trayMenu->addAction("Settings", this, SLOT(settingsAction()));
    trayMenu->addAction("About", this, SLOT(aboutAction()));
    trayMenu->addAction("Exit", this, SLOT(exitAction()));
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
    parent->showLogoutDialog();
}
