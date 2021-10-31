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

#include "note_formatter.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "helpers/xml_helpers.h"


NoteFormatter::NoteFormatter(QString input_text) : inputText(input_text) {

};

QString NoteFormatter::standardiseInput()
{
    QDomDocument doc;
    doc.setContent(inputText);

    QDomElement body = doc.elementsByTagName("en-note").at(0).toElement();

    // If note contains nothing, return empty ENML note.
    // -->  This ensures <en-note> has a child div so that initial user text
    //      is inserted in the correct place (not in the <body>)
    if(body.childNodes().size() <= 0) {
        return createEmptyENML();
    }

    QString output_str;
    QTextStream stream(&output_str);
    body.save(stream, 0);

    return enmlXmlHeader + output_str;
}

QString NoteFormatter::createEmptyENML()
{
    return enmlXmlHeader + "<en-note>\n<div>\n<br clear=\"none\"/>\n</div>\n</en-note>";
}

/*
 *  Convert ENML (XML) note content to valid HTML
 */
QString NoteFormatter::convertFromEML()
{
    QDomDocument doc;
    doc.setContent(inputText);

    // Convert ENML "en-note" top-level element to a div.
    QDomElement body = doc.elementsByTagName("en-note").at(0).toElement();
    body.setTagName("div");
    body.setAttribute("id", "en-note");

    // Convert ENML "en-todo" elements to html checkboxes.
    QDomNodeList todoElements = doc.elementsByTagName("en-todo");
    while(todoElements.size() > 0) {
        QDomElement todoElement = todoElements.at(0).toElement();

        todoElement.setTagName("input");
        todoElement.setAttribute("type", "checkbox");

        // Only keep 'checked' attribute if true, inferred false otherwise.
        QString checked = todoElement.attribute("checked");
        if(checked != "true") {
            todoElement.removeAttribute("checked");
        }
    }

    // Convert ENML "en-media" elements to html placeholders
    QDomNodeList mediaElements = doc.elementsByTagName("en-media");
    while(mediaElements.size() > 0) {
        QDomElement mediaElement = mediaElements.at(0).toElement();

        mediaElement.setTagName("img");
        mediaElement.setAttribute("class", "en-media unsupported");

        for(int i = 0; i < validMediaAttributes.size(); i++) {
            QString attribute_name = validMediaAttributes.at(i);
            QString attribute_value = mediaElement.attribute(validMediaAttributes.at(i));
            if(!attribute_value.isNull()) {
                mediaElement.setAttribute("data-" + attribute_name, attribute_value);
                mediaElement.removeAttribute(attribute_name);
            }
        }
    }

    // Convert ENML "en-crypt" elements to html placeholders
    QDomNodeList cryptElements = doc.elementsByTagName("en-crypt");
    while(cryptElements.size() > 0) {
        QDomElement cryptElement = cryptElements.at(0).toElement();

        cryptElement.setTagName("img");
        cryptElement.setAttribute("class", "en-crypt unsupported");

        for(int i = 0; i < validCryptAttributes.size(); i++) {
            QString attribute_name = validCryptAttributes.at(i);
            QString attribute_value = cryptElement.attribute(validCryptAttributes.at(i));
            if(!attribute_value.isNull()) {
                cryptElement.setAttribute("data-" + attribute_name, attribute_value);
                cryptElement.removeAttribute(attribute_name);
            }
        }
    }

    // Save to string
    QString output_str;
    QTextStream stream(&output_str);
    body.save(stream, 0);

    return output_str;
}

/*
 *  Convert HTML note content to ENML (XML)
 */
QString NoteFormatter::convertToEML()
{
    QDomDocument doc;
    doc.setContent(inputText);

    // Get parent element
    QDomElement parent = helpers::xml::findFirstElementOfTagMatchingAttribute("div", "id", "en-note", QList<QDomElement>() << doc.documentElement());
    parent.setTagName("en-note");
    helpers::xml::deleteAllAttributes(parent);

    QList<QDomElement> elements;
    helpers::xml::findAllElements(parent, elements);
    // Loop through and check each element
    for(int i = 0; i < elements.size(); i++) {
        QDomElement element = elements.at(i);

        // Convert ENML "en-todo" elements to html checkboxes.
        if(element.tagName() == "input" && element.attribute("type") == "checkbox") {
            element.setTagName("en-todo");
            element.removeAttribute("type");
        }

        // Convert ENML "en-media" elements to html placeholders
        if(element.tagName() == "img" && element.attribute("class").contains("en-media")) {
            element.setTagName("en-media");
            for(int i = 0; i < validMediaAttributes.size(); i++) {
                QString attribute_name = validMediaAttributes.at(i);
                QString attribute_value = element.attribute("data-" + attribute_name);
                if(!attribute_value.isNull()) {
                    element.setAttribute(attribute_name, attribute_value);
                    element.removeAttribute("data-" + attribute_name);
                }
            }
            helpers::xml::deleteAllChildren(element);
        }

        // Convert ENML "en-crypt" elements to html placeholders
        if(element.tagName() == "img" && element.attribute("class").contains("en-crypt")) {
            element.setTagName("en-crypt");
            for(int i = 0; i < validCryptAttributes.size(); i++) {
                QString attribute_name = validCryptAttributes.at(i);
                QString attribute_value = element.attribute("data-" + validCryptAttributes.at(i));
                if(!attribute_value.isNull()) {
                    element.setAttribute(attribute_name, attribute_value);
                    element.removeAttribute("data-" + attribute_name);
                }
            }
            helpers::xml::deleteAllChildren(element);
        }

        // Remove invalid attributes
        QStringList attributeNames = helpers::xml::getAllAttributeNames(element);
        QSet<QString> deleteAttributes = attributeNames.toSet().intersect(invalidAttributes.toSet());
        for(QSet<QString>::iterator i = deleteAttributes.begin(); i != deleteAttributes.end(); ++i) {
            element.removeAttribute(*i);
        }

        // Remove invalid tags
        if(!validTags.contains(element.tagName())) {
            helpers::xml::deleteNode(element);
        }
    }

    // Save to string
    QString output_str;
    QTextStream stream(&output_str);
    parent.save(stream, 0);

    output_str = enmlXmlHeader + output_str;

    return output_str;
}
