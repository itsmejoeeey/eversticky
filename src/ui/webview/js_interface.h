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

#ifndef JS_INTERFACE_H
#define JS_INTERFACE_H

#include <QDebug>
#include <QObject>


class NoteWebview;

class JsInterface: public QObject
{
Q_OBJECT

public:
    JsInterface(NoteWebview* parent);

    /// Log, for debugging
    Q_INVOKABLE void log(const QString& str) const;
    void setInnerHtml(QString text);

public slots:
    void domChanged(QString newText);
    void domHeightResized(int newDomHeight);
    void domLoaded();
    void domNewlineInserted(int caretY);

signals:
    void textUpdated(QString newText);
    void updateInnerHtml(QString text);
    void pageHeightUpdated(int newHeight);
    void pageLoaded();
    void ensureWebviewCaretVisible(int cursorY);

private:
    NoteWebview* m_Parent;
};

#endif // JS_INTERFACE_H
