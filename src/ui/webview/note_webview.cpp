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

    installEventFilter(this);

    openBlankPage();
}

void NoteWebview::updatePageHeight(int newHeight)
{
    int pageBottomPadding = 16;
    this->setFixedHeight(newHeight + pageBottomPadding);
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

    // Load note keyhandler js resource
    QFile jsKeyHandlerFile(":/ui/webview/note_webview_keyhandler.js");
    if(!jsKeyHandlerFile.open(QIODevice::ReadOnly))
        qCritical() << "Error loading note_webview_keyhandler.js to inject into webview...";
    QString jsKeyHandlerScript = QString::fromLatin1(jsKeyHandlerFile.readAll());

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
                <script>" + jsKeyHandlerScript + "</script> \
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

bool NoteWebview::eventFilter(QObject*, QEvent *event)
{
    // Would have rather used wheelEvent(), but on some Qt versions (such as Qt 5.12.8 default on Ubuntu 20.04)
    // wheel events aren't caught by the QWebEngineView itself. Need to attach an event filter to child QWidgets that appear
    // during page load.
    // https://bugreports.qt.io/browse/QTBUG-43602

    if (event->type() == QEvent::ChildAdded)
    {
        // Attach event filter to all children as they appear
        QChildEvent *e = static_cast<QChildEvent *>(event);
        QObject *child = e->child();
        child->installEventFilter(this);

        return true;
    }

    if (event->type() == QEvent::Wheel)
    {
        // Consume wheel event so it can be handled by NoteScrollArea
        return true;
    }

    return false;
}
