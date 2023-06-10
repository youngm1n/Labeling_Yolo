#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>

#include "object.h"
#include "dialogobjectclasseditor.h"

class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = nullptr);
    
    void loadImage(QString imgPath, OBJECTS objs);
    void selectObject(int objNo);
    void changeClassNo(int objNo, int classNo);
    void removeObject(int objNo);

public slots:
    void updateClassInformation(QStringList classList, CLASS_COLORS classColors);

private:
    QRectF *getScreenRectFromYoloRect(QRectF rectYolo);
    void moveDrawRectCenter(QPointF center);

private slots:
    void createNewObject(int classNo);

signals:
    void updateObjectsFromImageViewer(OBJECTS objs);

private:
    QImage img;
    OBJECTS objs;

    // Object's class params
    QStringList objClassNames;
    CLASS_COLORS objClassColors;

    // Draw tools
    QPointF currentMousePos;
    QRectF rectOriginalDraw;
    QRectF rectDraw;
    double zoomRatio;
    bool dragImg;
    bool dragResizer;
    QPointF dragStart;
    QPointF dragDelta;

    // Create new object
    bool newObj;
    QPointF posNewObj;
    QRectF rectNewObj;
    QComboBox *newObjClassSelctor;

    // Number of selected object and resizer
    int selObjNo;
    int selResizerNo;

    // QWidget interface
protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
};

#endif // IMAGEVIEWER_H
