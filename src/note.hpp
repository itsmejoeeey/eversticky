#ifndef NOTE_H
#define NOTE_H

#include <QEverCloud.h>

class Note
{
public:
    Note(qevercloud::Guid guid, int usn, QString title, QString content);
    Note();

    qevercloud::Guid guid;
    int usn;
    QString title;
    QString content;
    bool changed;
    bool new_note;
};

#endif // NOTE_H
