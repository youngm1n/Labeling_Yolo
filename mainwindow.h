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
    void pressedImageTableItem(QTableWidgetItem *item);
    void pressedLabelTableItem(QTableWidgetItem *item);

private:
    void loadObjectInfo(const QFileInfo &labelFileInfo, OBJECTS &objs);
    void initObjectTable(OBJECTS &);

private:
    Ui::MainWindow *ui;

    QFileInfoList imgFileInfos;
    QFileInfoList labelFileInfos;

    QList<QPushButton *> btnRemoveImg;

    ImageViewer *imgView;
    QTimer timerImgLoad;

    QSet<int> objClassSet;

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
