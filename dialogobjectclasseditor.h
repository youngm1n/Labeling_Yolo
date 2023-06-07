#ifndef DIALOGOBJECTCLASSEDITOR_H
#define DIALOGOBJECTCLASSEDITOR_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class DialogObjectClassEditor;
}

class DialogObjectClassEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogObjectClassEditor(QWidget *parent = nullptr);
    ~DialogObjectClassEditor();

    void initClassCount(int count);

private:
    QPushButton *getColorButton(int row);

private slots:
    void objectClassNameChanged(QTableWidgetItem *item);
    void classColorChanged(const QColor &color);

private:
    Ui::DialogObjectClassEditor *ui;
};

#endif // DIALOGOBJECTCLASSEDITOR_H
