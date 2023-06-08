#ifndef DIALOGOBJECTCLASSEDITOR_H
#define DIALOGOBJECTCLASSEDITOR_H

#include <QDialog>
#include <QTableWidgetItem>

typedef QList<QColor> CLASS_COLORS;

namespace Ui {
class DialogObjectClassEditor;
}

class DialogObjectClassEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogObjectClassEditor(QWidget *parent = nullptr);
    ~DialogObjectClassEditor();

    void clear();
    void insertNewClassNo(const int &no);

private:
    QPushButton *getColorButton(int row);
    QPushButton *getColorButton(int row, QColor color);

private slots:
    void objectClassNameChanged(QTableWidgetItem *item);
    void classColorChanged(const QColor &color);

signals:
    void updateClassInformation(QStringList classList, CLASS_COLORS classColors);

private:
    Ui::DialogObjectClassEditor *ui;
    QSet<int> objClassSet;
    QStringList objClassList;
    CLASS_COLORS objClassColors;

    // QDialog interface
public slots:
    int exec();
    void closeEvent(QCloseEvent *event);
};

#endif // DIALOGOBJECTCLASSEDITOR_H
