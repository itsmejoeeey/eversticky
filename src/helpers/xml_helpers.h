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

#ifndef XML_HELPERS_H
#define XML_HELPERS_H

#include <QDomElement>
#include <QString>


namespace helpers
{
    namespace xml
    {
        QDomElement findFirstElementOfTagMatchingAttribute(const QString& tag_name, const QString& attribute_name, const QString& attribute_value, QList<QDomElement> elems);
        void findAllElements(const QDomElement elem, QList<QDomElement>& foundElements);

        void deleteAllAttributes(QDomElement& element);
        void deleteAllChildren(QDomNode& node);
        void deleteNode(QDomNode& node);

        QStringList getAllAttributeNames(QDomElement& element);

    } // namespace xml
} // namespace misc

#endif // XML_HELPERS_H
