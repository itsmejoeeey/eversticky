#ifndef NOTEUI_H
#define NOTEUI_H

#include <QWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QTextEdit>

#include "note.hpp"
#include "cache.hpp"

#include "ui/content_text_edit.hpp"

class NoteController;

class NoteUI : public QWidget
{
Q_OBJECT
public:
    NoteUI(NoteController *parent, Note* note, noteItem size);
    void testFunc();
    void updateUI();
    void titleTextChanged();
    void contentTextChanged(QString newText);
    qevercloud::Guid getGUID();
    void updateModel(Note *note);
    void syncModel();
    Note* note;
    void bringToForeground();
    void setNoteSize(noteItem size);
private slots:
    void killWindow();
    void addWindow();
private:
    NoteController *parent;
    ContentTextEdit * contentEdit;
    HeaderBlock * headerBlock;

    bool initialized;

    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // NOTE_H
