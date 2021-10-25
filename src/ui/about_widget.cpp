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

#include "about_widget.h"

#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>


AboutWidget::AboutWidget() : QDialog()
{
    setObjectName("AboutWindow");
    setWindowTitle("EverSticky | About");

    QIcon icon(":/icon/appicon.ico");
    this->setWindowIcon(icon);

    // Read and apply note stylesheet
    QFile File(":style/about_stylesheet.qss");
    File.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(File.readAll());
    setStyleSheet(styleSheet);


    // Parent layout
    QVBoxLayout *parentLayout = new QVBoxLayout(this);
    parentLayout->setAlignment(Qt::AlignCenter);
    parentLayout->setContentsMargins(50,50,50,50);
    parentLayout->setSpacing(25);

    QPixmap appLogoPixmap = icon.pixmap(128);

    QLabel *appLogo = new QLabel(this);
    appLogo->setPixmap(appLogoPixmap);
    QLabel *appTitle = new QLabel("<h1>EverSticky</h1>", this);
    QLabel *appVersion = new QLabel("<h3>Version " + QString::fromStdString(APP_VERSION) + "</h3>");
    QLabel *appSubtitle = new QLabel("This project is open source and contributions are welcomed.");
    QLabel *appDescription = new QLabel("Visit <a href='https://github.com/itsmejoeeey/eversticky'>https://github.com/itsmejoeeey/eversticky</a> for more information or to report a bug or to suggest a new feature.");
    appDescription->setOpenExternalLinks(true);
    QLabel *appCopyright = new QLabel("Copyright © 2021 Joey Miller. See 'LICENSE' included with application source for license terms.");

    // Add all widgets to the dialog
    parentLayout->addWidget(appLogo);
    parentLayout->addWidget(appTitle);
    parentLayout->addWidget(appVersion);
    parentLayout->addWidget(appSubtitle);
    parentLayout->addWidget(appDescription);
    parentLayout->addWidget(appCopyright);

    show();
}
