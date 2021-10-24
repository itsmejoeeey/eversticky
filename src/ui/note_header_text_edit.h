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

#ifndef NOTE_HEADER_TEXT_EDIT_H
#define NOTE_HEADER_TEXT_EDIT_H

#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "note_header.h"


class NoteHeaderTextEdit : public QPlainTextEdit
{
Q_OBJECT

public:
    NoteHeaderTextEdit(QWidget *parent);

    void setText(QString text);

signals:
    void textUpdated(QString newText);

private slots:
    void textChanged();

private:
    QString getText();

    void resizeEvent(QResizeEvent *event) override;
    void resizeToFitContent();

protected:
    bool eventFilter(QObject *object, QKeyEvent *event);
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // NOTE_HEADER_TEXT_EDIT_H
