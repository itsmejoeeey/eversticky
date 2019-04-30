TEMPLATE = app
TARGET = eversticky

greaterThan(QT_MAJOR_VERSION, 4): QT += core gui network sql webenginewidgets widgets x11extras

SOURCES += \
    main.cpp \
    ui/content_text_edit.cpp \
    ui/header_text_edit.cpp \
    ui/header_block.cpp \
    cache.cpp \
    ui/note_ui.cpp \
    note.cpp \
    misc.cpp \
    note_controller.cpp \
    ui/tray_ui.cpp

HEADERS += \
    ui/content_text_edit.hpp \
    ui/header_block.hpp \
    ui/header_text_edit.hpp \
    ui/note_ui.hpp \
    note.hpp \
    misc.hpp \
    cache.hpp \
    note_controller.hpp \
    ui/tray_ui.hpp

DISTFILES += \
    style/note_stylesheet.qss \

RESOURCES += \
    resources.qrc

LIBS += \
    -L$$PWD/lib -lqt5qevercloud_linux64 \
    -lX11

INCLUDEPATH += \
    lib/qt5qevercloud_headers
