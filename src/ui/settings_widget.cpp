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

#include "settings_widget.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>


#include "settings.h"


SettingsWidget::SettingsWidget() : QDialog()
{
    setObjectName("SettingsWindow");
    setWindowTitle("EverSticky | Settings");

    QIcon icon(":/icon/appicon.ico");
    this->setWindowIcon(icon);


    /*
     *  First settings group - general settings
     */
    QGroupBox *generalGroupBox = new QGroupBox("General settings", this);

    QGridLayout *generalGridLayout = new QGridLayout(generalGroupBox);

    QLabel *syncIntervalLabel = new QLabel("Sync interval (seconds)", this);
    syncIntervalInput = new QSpinBox(this);
    syncIntervalInput->setMinimum(30);
    syncIntervalInput->setMaximum(100000);
    // Set initial spinbox state
    syncIntervalInput->setValue(Settings::getUserSetting("sync_interval").toInt());

    QLabel *checkUpdatesLabel = new QLabel("Check for updates", this);
    checkUpdatesCheckbox = new QCheckBox(this);
    // Set initial checkbox state
    if(Settings::getUserSetting("check_for_updates").toBool())
        checkUpdatesCheckbox->setCheckState(Qt::Checked);
    else
        checkUpdatesCheckbox->setCheckState(Qt::Unchecked);

    generalGridLayout->addWidget(syncIntervalLabel, 0, 0);
    generalGridLayout->addWidget(syncIntervalInput, 0, 1);
    generalGridLayout->addWidget(checkUpdatesLabel, 1, 0);
    generalGridLayout->addWidget(checkUpdatesCheckbox, 1, 1);


    /*
     *  Second settings group - ui settings
     */
    QGroupBox *uiGroupBox = new QGroupBox("UI settings", this);

    QGridLayout *uiGridLayout = new QGridLayout(uiGroupBox);

    QLabel *trayIconStyleLabel = new QLabel("Tray icon style", this);
    trayIconStyleCombobox = new QComboBox(this);
    trayIconStyleCombobox->addItems({"Light", "Dark"});
    // Set initial combobox state
    trayIconStyleCombobox->setCurrentIndex((int)Settings::getUserSetting("dark_status_icon").toBool());

    uiGridLayout->addWidget(trayIconStyleLabel, 0, 0);
    uiGridLayout->addWidget(trayIconStyleCombobox, 0, 1);


    // Parent layout
    QVBoxLayout *parentLayout = new QVBoxLayout(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsWidget::saveSettings);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsWidget::exitSettings);

    // Add all widgets to the dialog
    parentLayout->addWidget(generalGroupBox);
    parentLayout->addWidget(uiGroupBox);
    parentLayout->addWidget(buttonBox);

    show();
}

void SettingsWidget::exitSettings()
{
    close();
}

void SettingsWidget::saveSettings()
{
    Settings::setUserSetting("sync_interval", syncIntervalInput->value());
    Settings::setUserSetting("check_for_updates", (bool)checkUpdatesCheckbox->checkState());
    Settings::setUserSetting("dark_status_icon", (bool)trayIconStyleCombobox->currentIndex());

    // Causes XCB error to show in application output on modal dialog close.
    // See unresolved Qt bug: https://bugreports.qt.io/browse/QTBUG-56893
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Information");
    msgBox.setText("Settings saved.");
    msgBox.setInformativeText("Restart EverSticky for changes to take effect.");
    msgBox.exec();

    close();
}
