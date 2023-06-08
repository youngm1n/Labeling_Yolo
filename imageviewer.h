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
    
    void setObjectClassInfomation(QStringList classList, CLASS_COLORS classColorList);
    void loadImage(QString imgPath, OBJECTS &objs);
    void updateObjects(OBJECTS &objs);

private:
    QRectF *getScreenRectFromYoloRect(QRectF rectYolo);
    void updateObjectScreenRect();
    void moveDrawRectCenter(QPointF center);

signals:

private:
    QImage img;

    // Object draw params
    QList<QRectF *> rectYoloObjs;
    QList<QRectF *> rectScrObjs;
    QList<QVector<QRectF *>> rectResizers;

    // Object's class params
    QStringList classList;
    CLASS_COLORS classColors;
    QList<int> objClass;

    // Draw tools
    QPointF mousePos;
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
