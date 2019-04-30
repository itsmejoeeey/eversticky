// Standard libs
#include <iostream>
// QT libs
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
// Project libs
#include "note_controller.hpp"


int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    const QRect screenSize = app.primaryScreen()->virtualGeometry();
    const int screens = app.screens().length();
    NoteController *noteController = new NoteController(screens, screenSize.width(), screenSize.height());

    return app.exec();
}


