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

#include "js_interface.h"

#include "note_webview.h"


JsInterface::JsInterface(NoteWebview* parent) : QObject(), m_Parent(parent)
{
}

void JsInterface::log(const QString& str) const
{
    qInfo() << "Message from js: " << str;
}

void JsInterface::domChanged(QString newText)
{
    emit textUpdated(newText);
}

void JsInterface::domHeightResized(int newDomHeight)
{
    emit pageHeightUpdated(newDomHeight);
}

void JsInterface::domNewlineInserted(int cursorY)
{
    emit ensureWebviewCaretVisible(cursorY);
}

void JsInterface::domLoaded()
{
    emit pageLoaded();
}

void JsInterface::setInnerHtml(QString text)
{
    emit updateInnerHtml(text);
}
