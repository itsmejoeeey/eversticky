TEMPLATE = app
TARGET = eversticky

DEFINES += \
    APP_VERSION=\\\"0.95.0\\\" \
    \
    ## API_DOMAIN
    # Can be either 'www.evernote.com' or 'sandbox.evernote.com' for testing
    # --> If error 'Host requires authentication' you must go to 'https://dev.evernote.com/support/'
    #       to request activation of your API key on our production service.
    API_HOST=\\\"sandbox.evernote.com\\\" \
    #
    ## API_KEY
    # Typically The username of the user than owns the API key
    API_KEY=\\\"joejoe31b-9118\\\" \
    #
    ## API_SECRET
    # The 16-digit API key
    API_SECRET=\\\"1f7f6b75e35e7369\\\"


greaterThan(QT_MAJOR_VERSION, 4): QT += core gui network sql webenginewidgets webkit webkitwidgets widgets x11extras xml

SOURCES += \
    main.cpp \
    helpers/misc_helpers.cpp \
    helpers/xml_helpers.cpp \
    note_formatter.cpp \
    cache.cpp \
    note_sync_controller.cpp \
    settings.cpp \
    ui/note_header.cpp \
    ui/note_header_text_edit.cpp \
    ui/note_scroll_area.cpp \
    ui/note_title_bar.cpp \
    note.cpp \
    note_controller.cpp \
    ui/note_widget.cpp \
    ui/settings_widget.cpp \
    ui/tray_icon.cpp \
    ui/webview/js_interface.cpp \
    ui/webview/note_webview.cpp \
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
    cache.h \
    helpers/misc_helpers.h \
    helpers/xml_helpers.h \
    note.h \
    note_controller.h \
    note_formatter.h \
    note_sync_controller.h \
    settings.h \
    ui/note_header.h \
    ui/note_header_text_edit.h \
    ui/note_scroll_area.h \
    ui/note_title_bar.h \
    ui/note_widget.h \
    ui/settings_widget.h \
    ui/tray_icon.h \
    ui/webview/js_interface.h \
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
    qevercloud/generated/types_impl.h \
    ui/webview/note_webview.h

DISTFILES += \
    style/note_stylesheet.qss

RESOURCES += \
    resources.qrc \
    ui/webview/note_webview.css \
    ui/webview/note_webview.js

LIBS += \
    -lX11

INCLUDEPATH += \
    qevercloud
