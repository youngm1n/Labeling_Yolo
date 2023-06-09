#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDir>
#include <QFileInfoList>
#include <QList>
#include <QPushButton>
#include <QTableWidgetItem>

#include "imageviewer.h"
#include "object.h"
#include "dialogobjectclasseditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImageFolder(QDir dirImage);
    void timoutLoadImageFile();
    void pressedImageRemoveButton();
    void pressedObjectRemoveButton();
    void pressedImageTableItem(QTableWidgetItem *item);
    void pressedLabelTableItem(QTableWidgetItem *item);
    void changedObjectClass(int newClassNo);
    void updateClassInformation(QStringList classList, CLASS_COLORS classColors);
    void updateObjects(OBJECTS objs);

private:
    void loadObjectInfo(const QFileInfo &labelFileInfo, OBJECTS &objs);
    void initObjectTable(OBJECTS &objs);
    void showImageThumbnailInTable();

private:
    Ui::MainWindow *ui;

    int selectedImgRow;

    QFileInfoList imgFileInfos;
    QFileInfoList labelFileInfos;

    QList<QPushButton *> btnRemoveImg;

    ImageViewer *imgEditor;
    QTimer timerImgLoad;

    DialogObjectClassEditor *objClassEditor;
    QStringList objClassNames;
    CLASS_COLORS objClassColors;

    // QWidget interface
protected:
    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
private slots:
    void on_toolButtonLoadImgFolder_clicked();
};
#endif // MAINWINDOW_H
