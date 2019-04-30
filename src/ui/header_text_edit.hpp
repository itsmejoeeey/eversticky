#ifndef HEADER_TEXT_EDIT_H
#define HEADER_TEXT_EDIT_H

#include "header_block.hpp"

#include <QVBoxLayout>
#include <QPlainTextEdit>


class HeaderTextEdit : public QPlainTextEdit
{
Q_OBJECT
public:
    HeaderTextEdit(QWidget *parent, HeaderBlock *parentController);
    void setText(QString text);
    QString getText();
private slots:
    void textChanged();
private:
    HeaderBlock *parent;
    void resizeEvent(QResizeEvent *event) override;
protected:
    bool eventFilter(QObject *object, QKeyEvent *event);
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // HEADER_TEXT_EDIT_H
