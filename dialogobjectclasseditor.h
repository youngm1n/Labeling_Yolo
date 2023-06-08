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
    void getClassInformation(QStringList &classList, CLASS_COLORS &classColors);
    const QStringList getClassList();
    const CLASS_COLORS getClassColors();

private:
    QPushButton *getColorButton(int row);

private slots:
    void objectClassNameChanged(QTableWidgetItem *item);
    void classColorChanged(const QColor &color);

private:
    Ui::DialogObjectClassEditor *ui;
    QSet<int> objClassSet;

    // QDialog interface
public slots:
    int exec();
};

#endif // DIALOGOBJECTCLASSEDITOR_H
