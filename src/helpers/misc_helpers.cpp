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

#include "misc_helpers.h"

#include <algorithm>


namespace helpers
{
    namespace misc
    {
        QString random_hex_string(unsigned int length)
        {
            const char charset[] = "0123456789"
                                   "abcdef";
            const unsigned char maxIndex = (sizeof(charset) - 1);

            std::string str(length,0);
            std::generate_n(str.begin(), length, [&]() -> char {
                return charset[rand() % maxIndex];
            });

            return QString::fromStdString(str);
        }
    } // namespace misc
} // namespace helpers