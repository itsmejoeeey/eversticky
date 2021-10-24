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

#ifndef NOTEFORMATTER_H
#define NOTEFORMATTER_H

#include <QString>
#include <QStringList>


static const QString enmlXmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
                        "<!DOCTYPE en-note SYSTEM \"http://xml.evernote.com/pub/enml2.dtd\">\n";

class NoteFormatter
{
public:
    NoteFormatter(QString input_text);

    QString standardiseInput();

    QString convertFromEML();
    QString convertToEML();

    static QString createEmptyENML();

private:
    QString inputText;

    QStringList invalidAttributes = { "id", "class", "onclick", "ondblclick", //"on*",
                                      "accesskey", "data", "dynsrc", "tabindex" };
    QStringList validCryptAttributes = { "cipher", "hint", "length" };
    QStringList validMediaAttributes = { "align", "hash", "style", "type" };

    QStringList validTags = { "en-note", "en-todo", "en-media", "en-crypt",
                              //
                              "a", "abbr", "acronym", "address", "area",
                              "b", "bdo", "big", "blockquote", "br", "caption",
                              "center", "cite", "code", "col", "colgroup", "dd",
                              "del", "dfn", "div", "dl", "dt", "em", "font", "h1",
                              "h2", "h3", "h4", "h5", "h6", "hr", "i", "img", "ins",
                              "kbd", "li", "map", "ol", "p", "pre", "q", "s", "samp",
                              "small", "span", "strike", "strong", "sub", "sup", "table",
                              "tbody", "td", "tfoot", "th", "thead", "title", "tr", "tt",
                              "u", "ul", "var", "xmp" };
};

#endif // NOTEFORMATTER_H
