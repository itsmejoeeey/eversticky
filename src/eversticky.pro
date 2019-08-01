TEMPLATE = app
TARGET = eversticky

DEFINES += \
    ## API_DOMAIN
    # Can be either 'www.evernote.com' or 'sandbox.evernote.com' for testing
    # --> If error 'Host requires authentication' you must go to 'https://dev.evernote.com/support/'
    #       to request activation of your API key on our production service.
    API_HOST=\\\"XXXXXXXXXXXXXXXX\\\" \
    #
    ## API_KEY
    # Typically The username of the user than owns the API key
    API_KEY=\\\"XXXXXXXXXXXXXXXX\\\" \
    #
    ## API_SECRET
    # The 16-digit API key
    API_SECRET=\\\"XXXXXXXXXXXXXXXX\\\"

greaterThan(QT_MAJOR_VERSION, 4): QT += core gui network sql webenginewidgets webkit webkitwidgets widgets x11extras

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
    ui/tray_ui.cpp \
#
    qevercloud/AsyncResult.cpp \
    qevercloud/EventLoopFinisher.cpp \
    qevercloud/EverCloudException.cpp \
    qevercloud/exceptions.cpp \
    qevercloud/globals.cpp \
    qevercloud/http.cpp \
    qevercloud/InkNoteImageDownloader.cpp \
    qevercloud/oauth.cpp \
    qevercloud/services_nongenerated.cpp \
    qevercloud/thumbnail.cpp \
    qevercloud/generated/constants.cpp \
    qevercloud/generated/services.cpp \
    qevercloud/generated/types.cpp

HEADERS += \
    ui/content_text_edit.hpp \
    ui/header_block.hpp \
    ui/header_text_edit.hpp \
    ui/note_ui.hpp \
    note.hpp \
    misc.hpp \
    cache.hpp \
    note_controller.hpp \
    ui/tray_ui.hpp \
#
    qevercloud/AsyncResult.h \
    qevercloud/EventLoopFinisher.h \
    qevercloud/EverCloudException.h \
    qevercloud/exceptions.h \
    qevercloud/export.h \
    qevercloud/globals.h \
    qevercloud/http.h \
    qevercloud/impl.h \
    qevercloud/InkNoteImageDownloader.h \
    qevercloud/oauth.h \
    qevercloud/Optional.h \
    qevercloud/QEverCloud.h \
    qevercloud/QEverCloudOAuth.h \
    qevercloud/qt4helpers.h \
    qevercloud/thrift.h \
    qevercloud/thumbnail.h \
    qevercloud/generated/constants.h \
    qevercloud/generated/EDAMErrorCode.h \
    qevercloud/generated/services.h \
    qevercloud/generated/types.h \
    qevercloud/generated/types_impl.h

DISTFILES += \
    style/note_stylesheet.qss \

RESOURCES += \
    resources.qrc

LIBS += \
    -lX11

INCLUDEPATH += \
    qevercloud
