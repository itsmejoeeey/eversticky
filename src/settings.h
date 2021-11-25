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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QStandardPaths>
#include <QString>
#include <QVariant>


class Settings
{
public:
    static void deleteAllSessionSettings();
    static void deleteCurrentSessionSettings();
    static QString getSessionSetting(QString settingKey);
    static void setSessionSetting(QString settingKey, QString settingValue);

    static QVariant getUserSetting(QString settingKey);
    static void setUserSetting(QString settingKey, QVariant settingValue);

private:
    static QString getSessionSettingsFileLocation();
    static QString getUserSettingsFileLocation();
};

#endif // SETTINGS_H
