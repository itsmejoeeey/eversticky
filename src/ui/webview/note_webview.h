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

#ifndef NOTE_WEBVIEW_H
#define NOTE_WEBVIEW_H

#include <QWebEngineView>

#include "js_interface.h"
#include "ui/note_header.h"


class NoteWebview : public QWebEngineView
{
Q_OBJECT

public:
    NoteWebview(QWidget *context);

    void clearText();
    void setText(QString text);
    QString getText();

public slots:
    void updatePageHeight(int newHeight);

signals:
    void textUpdated(QString newText);
    void initialNoteLoad();
    void ensureWebviewCaretVisible(int caretY);
    void noteLoaded();

private:
    QWebChannel* channel;
    JsInterface* jsInterface;

    void openBlankPage();

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // NOTE_WEBVIEW_H
