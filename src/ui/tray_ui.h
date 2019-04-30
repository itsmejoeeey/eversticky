#ifndef TRAY_UI_H
#define TRAY_UI_H

#include <QSystemTrayIcon>
#include "note_controller.hpp"


class TrayUI : public QSystemTrayIcon
{
Q_OBJECT
public:
    TrayUI(NoteController* parent);
private slots:
    void trayEvent(QSystemTrayIcon::ActivationReason reason);

    void loginAction();
    void createAction();
    void foregroundAction();
    void syncAction();
    void exitAction();
    void logout();
private:
    NoteController *parent;
    void createIconMenu(QPoint point);
};

#endif // TRAY_UI_H
