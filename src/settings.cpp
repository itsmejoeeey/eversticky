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

#include "settings.h"

#include <QDebug>
#include <QFile>
#include <QSettings>


static QMap<QString, QVariant> defaultUserSettings =
{
    {"sync_interval", 300},
    {"check_for_updates", true},
    {"dark_status_icon", false}
};

QString Settings::getSessionSettingsFileLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).append("/session_settings.ini");
}

QString Settings::getUserSettingsFileLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).append("/user_settings.ini");
}

void Settings::deleteAllSessionSettings()
{
    QFile(getSessionSettingsFileLocation()).remove();
}

void Settings::deleteCurrentSessionSettings()
{
    // Delete all session settings except for the session username.
    // This allows us to avoid deleting all unsynced notes if the same user is logging in again (i.e. after having to login again
    // after Evernote downtime, or auth key expiring).
    QString currentUsername = getSessionSetting("username");
    deleteAllSessionSettings();
    setSessionSetting("username", currentUsername);
}

QString Settings::getSessionSetting(QString settingKey)
{
    QSettings settings(getSessionSettingsFileLocation(), QSettings::NativeFormat);
    return settings.value(settingKey).toString();
}

void Settings::setSessionSetting(QString settingKey, QString settingValue)
{
    QSettings settings(getSessionSettingsFileLocation(), QSettings::NativeFormat);
    settings.setValue(settingKey, settingValue);
}

QVariant Settings::getUserSetting(QString settingKey)
{
    QSettings settings(getUserSettingsFileLocation(), QSettings::NativeFormat);

    if(settings.contains(settingKey)) {
        return settings.value(settingKey);
    } else {
        // If settings value doesn't exist, return default value.
        return defaultUserSettings.value(settingKey);
    }
}

void Settings::setUserSetting(QString settingKey, QVariant settingValue)
{
    QSettings settings(getUserSettingsFileLocation(), QSettings::NativeFormat);
    settings.setValue(settingKey, settingValue);
}
