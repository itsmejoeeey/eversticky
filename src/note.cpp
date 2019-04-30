#include "note.hpp"

// Standard libs
#include <iostream>


Note::Note(qevercloud::Guid guid, int usn, QString title, QString content) :
    guid(guid), usn(usn), title(title), content(content)
{
    changed = false;
    new_note = false;
}

Note::Note() {
    changed = false;
    new_note = true;
}
