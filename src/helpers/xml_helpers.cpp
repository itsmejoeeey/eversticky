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

#include "xml_helpers.h"

#include <QDebug>


namespace helpers
{
    namespace xml
    {
        QDomElement findFirstElementOfTagMatchingAttribute(const QString& tag_name, const QString& attribute_name, const QString& attribute_value, QList<QDomElement> elems)
        {
            QList<QDomElement> next_elems = QList<QDomElement>();

            for(int i = 0; i < elems.size(); i++) {
                QDomElement elem = elems.at(i);
                if( elem.tagName() == tag_name && elem.attribute(attribute_name) == attribute_value)
                  return elem;

                QDomNodeList append_list = elem.childNodes();
                for(int n = 0; n < append_list.size(); n++) {
                    next_elems.append(append_list.at(n).toElement());
                }
            }

            return findFirstElementOfTagMatchingAttribute(tag_name, attribute_name, attribute_value, next_elems);
        }

        void findAllElements(const QDomElement elem, QList<QDomElement>& foundElements)
        {
          foundElements.append(elem);

          QDomElement child = elem.firstChildElement();
          while(!child.isNull()) {
            findAllElements(child, foundElements);
            child = child.nextSiblingElement();
          }
        }

        void deleteAllAttributes(QDomElement& element)
        {
            while(element.attributes().size() > 0) {
                QDomAttr attribute = element.attributes().item(0).toAttr();
                element.removeAttribute(attribute.name());
            }
        }

        void deleteAllChildren(QDomNode& node) {
            QDomNodeList childNodes = node.childNodes();
            while(!childNodes.isEmpty()) {
                QDomNode childNode = childNodes.at(0);
                deleteNode(childNode);
            }
        }

        void deleteNode(QDomNode& node)
        {
            node.parentNode().removeChild(node);
        }

        QStringList getAllAttributeNames(QDomElement& element)
        {
            QStringList output;
            QDomNamedNodeMap attributes = element.attributes();
            for(int i = 0; i < attributes.size(); i++) {
                output.append(attributes.item(i).toAttr().name());
            }

            return output;
        }
    } // namespace xml
} // namespace helpers
