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

#include "note_webview.h"

#include <iostream>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDropEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QThread>
#include <QWebChannel>
#include <QWebEngineSettings>

#include "js_interface.h"
#include "note_formatter.h"
#include "ui/note_widget.h"


NoteWebview::NoteWebview(QWidget *context) : QWebEngineView(context)
{
    setObjectName("NoteWebview");

    setFixedHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    page()->setBackgroundColor(Qt::transparent);
    settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    // Create channel to communicate with the webview
    channel = new QWebChannel(this);
    this->page()->setWebChannel(channel);
    jsInterface = new JsInterface(this);
    channel->registerObject(QString("parentWebEngine"), jsInterface);

    // When webview javascript indicates page is loaded, emit another signal.
    // This signal indicates the webview is ready to be sent content to display (via setText())
    connect(jsInterface, &JsInterface::pageLoaded, this, &NoteWebview::initialNoteLoad);

    connect(jsInterface, &JsInterface::pageHeightUpdated, this, &NoteWebview::updatePageHeight);
    connect(jsInterface, &JsInterface::textUpdated, this, &NoteWebview::textUpdated);
    connect(jsInterface, &JsInterface::ensureWebviewCaretVisible, this, &NoteWebview::ensureWebviewCaretVisible);

    openBlankPage();
}

void NoteWebview::updatePageHeight(int newHeight)
{
    this->setFixedHeight(newHeight);
}

void NoteWebview::openBlankPage()
{
    QFile apiFile(":/qtwebchannel/qwebchannel.js");
    if(!apiFile.open(QIODevice::ReadOnly))
        qCritical() << "Error loading qwebchannel.js to inject into webview...";
    QString apiScript = QString::fromLatin1(apiFile.readAll());
    apiFile.close();
    this->page()->runJavaScript(apiScript);

    // Load note js resource
    QFile jsFile(":/ui/webview/note_webview.js");
    if(!jsFile.open(QIODevice::ReadOnly))
        qCritical() << "Error loading note_webview.js to inject into webview...";
    QString jsScript = QString::fromLatin1(jsFile.readAll());
    this->page()->runJavaScript(jsScript);

    // Load note css resource
    QFile cssFile(":/ui/webview/note_webview.css");
    if(!cssFile.open(QIODevice::ReadOnly))
        qCritical() << "Error loading note_webview.css to inject into webview...";
    QString noteCss = QString::fromLatin1(cssFile.readAll());

    setHtml("\
        <html> \
            <head>\
                <script>" + apiScript + "</script> \
                <script>" + jsScript + "</script> \
                <style>" + noteCss + "</style> \
            </head> \
            <body></body>\
        </html>"
    );
}

// Don't call until first page load is finished.
void NoteWebview::setText(QString text)
{
    jsInterface->setInnerHtml(text);
}


void NoteWebview::wheelEvent(QWheelEvent *event)
{
    QPoint amount = event->pixelDelta();
    emit scrollEvent(amount.rx(), amount.ry());

    event->ignore();
}
