#ifndef HEADER_BLOCK_H
#define HEADER_BLOCK_H

#include <QTextEdit>
#include <QHBoxLayout>





class ContentTextEdit;
class HeaderTextEdit;

class NoteUI;

class HeaderBlock : public QWidget
{
Q_OBJECT
public:
    HeaderBlock(ContentTextEdit* parent, NoteUI *context);
    //void updateLeftColumnLayout();
    void setText(QString text);
    void clearText();
    QString getText();
    void textChanged();
   private:
    QVBoxLayout *left_column_layout;
    QVBoxLayout *main_layout;
    QWidget *left_column_frame;
    HeaderTextEdit *header_text;
    NoteUI *context;

    ContentTextEdit *parent;
    bool textSet;
public:
    void setScrollPosition(int newScrollPosition);
    //void resizeEvent(QResizeEvent *event) override;
    int getHeight();
};

#endif // HEADER_BLOCK_H
