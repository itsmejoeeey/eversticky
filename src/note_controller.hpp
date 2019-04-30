#ifndef NOTECONTROLLER_H
#define NOTECONTROLLER_H

#include <QWidget>
#include <QMainWindow>
#include <QEverCloud.h>
#include <QSettings>
#include <QObject>

#include <vector>
#include "note.hpp"
#include "cache.hpp"

class NoteUI;

class NoteController : public QObject
{
Q_OBJECT
public:
    NoteController(int screens, int screenWidth, int screenHeight);
    NoteUI* createNote();
    NoteUI* createNote(Note *note);
    NoteUI* createNote(Note* note_model, noteItem size);
    void syncChanges();
    void syncFromServer();
    void syncAllNoteModels();
    QString getUsername();
    QString state;

    void bringAllToFront();
    void updateNoteDimensions(qevercloud::Guid guid, int x, int y, int width, int height);

    void logout();
public slots:
    bool buttonClicked();
    void button2Clicked();
    void button3Clicked();
    void button4Clicked();
    void periodicUpdate();
private:
    std::vector<NoteUI*> notes;
    int noteCount;
    qevercloud::Guid notebookGUID;
    QString settingsFile;

    int screens;
    int screenWidth;
    int screenHeight;


};

#endif // NOTECONTROLLER_H
