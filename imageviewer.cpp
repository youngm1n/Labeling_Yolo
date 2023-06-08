#include "imageviewer.h"

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

enum ENUM_SIZE_MOD  { RESIZER_TOP_LEFT, RESIZER_TOP_RIGHT, RESIZER_BOTTOM_LEFT, RESIZE_BOTTOM_RIGHT, RESIZER_TOTAL };

#define RESIZER_RADIUS    7

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget{parent}
{
    zoomRatio = 1;
    dragImg = dragResizer = newObj = false;

    selObjNo = selResizerNo = -1;

    qApp->installEventFilter(this);
}

void ImageViewer::setObjectClassInfomation(QStringList list, CLASS_COLORS colors)
{
    classList = list;
    classColors = colors;
}

void ImageViewer::loadImage(QString imgPath, OBJECTS &objs)
{
    // Init draw parameters

    // Load image
    img.load(imgPath);

    // Set original rect for drawing
    auto ratio = img.width() / static_cast<float>(img.height());
    rectOriginalDraw.setHeight(height());
    rectOriginalDraw.setWidth(height() * ratio);
    if (rectOriginalDraw.width() > width()) {
        rectOriginalDraw.setWidth(width());
        rectOriginalDraw.setHeight(width() / ratio);
    }
    rectOriginalDraw.moveCenter(rect().center());
    rectDraw = rectOriginalDraw;

    // Update objects
    updateObjects(objs);

    // Update screen
    update();
}

void ImageViewer::updateObjects(OBJECTS &objs)
{
    // Clear params
    rectYoloObjs.clear();
    rectResizers.clear();
    rectScrObjs.clear();
    objClass.clear();

    foreach (auto obj, objs) {
        rectYoloObjs.push_back(new QRectF(obj->getRect()));
        auto scrRect = getScreenRectFromYoloRect(obj->getRect());
        rectScrObjs.push_back(scrRect);
        objClass.push_back(obj->getClassNo());

        QList<QRectF*> mod;
        for (int i = 0; i < RESIZER_TOTAL; i++) {
            auto r = new QRectF(0, 0, RESIZER_RADIUS, RESIZER_RADIUS);
            mod.push_back(r);

            switch (i) {
            case RESIZER_TOP_LEFT:     r->moveCenter(scrRect->topLeft());      break;
            case RESIZER_TOP_RIGHT:    r->moveCenter(scrRect->topRight());     break;
            case RESIZER_BOTTOM_LEFT:  r->moveCenter(scrRect->bottomLeft());   break;
            default:                   r->moveCenter(scrRect->bottomRight());  break;
            }
        }

        rectResizers.push_back(mod);
    }

    update();
}

QRectF *ImageViewer::getScreenRectFromYoloRect(QRectF rectYolo)
{
    auto scrRect = new QRectF(0, 0, rectYolo.width() * rectDraw.width(), rectYolo.height() * rectDraw.height());
    scrRect->moveTopLeft(QPointF(rectYolo.left() * rectDraw.width(), rectYolo.top() * rectDraw.height()) + rectDraw.topLeft());
    return scrRect;
}

// Update Object's screen rect when screen zoom or move
void ImageViewer::updateObjectScreenRect()
{
    for (int i = 0; i < rectYoloObjs.count(); i++) {
        auto rectScrObj = rectScrObjs.at(i);
        auto rectYoloObj = rectYoloObjs.at(i);
        rectScrObj->setSize(QSizeF(rectYoloObj->width() * rectDraw.width(), rectYoloObj->height() * rectDraw.height()));
        rectScrObj->moveTopLeft(QPointF(rectYoloObj->left() * rectDraw.width(), rectYoloObj->top() * rectDraw.height()) + rectDraw.topLeft());

        rectResizers.at(i).at(RESIZER_TOP_LEFT)->moveCenter(rectScrObj->topLeft());
        rectResizers.at(i).at(RESIZER_TOP_RIGHT)->moveCenter(rectScrObj->topRight());
        rectResizers.at(i).at(RESIZER_BOTTOM_LEFT)->moveCenter(rectScrObj->bottomLeft());
        rectResizers.at(i).at(RESIZE_BOTTOM_RIGHT)->moveCenter(rectScrObj->bottomRight());
    }
}

void ImageViewer::moveDrawRectCenter(QPointF center)
{
    rectDraw.moveCenter(center);
    if (rectDraw.left() > rectOriginalDraw.left()) {
        rectDraw.moveLeft(rectOriginalDraw.left());
    }
    if (rectDraw.right() < rectOriginalDraw.right()) {
        rectDraw.moveRight(rectOriginalDraw.right());
    }
    if (rectDraw.top() > rectOriginalDraw.top()) {
        rectDraw.moveTop(rectOriginalDraw.top());
    }
    if (rectDraw.bottom() < rectOriginalDraw.bottom()) {
        rectDraw.moveBottom(rectOriginalDraw.bottom());
    }
}


bool ImageViewer::eventFilter(QObject *watched, QEvent *event)
{
    // Get mouse move position
    if (watched == this && event->type() == QEvent::MouseMove) {
        bool cursonOn = false;
        auto tempMousePos = static_cast<QMouseEvent *>(event)->position();

        // Drag screen
        if (dragImg) {
            dragDelta = tempMousePos - dragStart;
            moveDrawRectCenter(rectDraw.center() + dragDelta);
            update();

            dragStart = tempMousePos;
        }
        // Drag object's resizer
        else if (dragResizer) {
            auto tempScrRect = *rectScrObjs.at(selObjNo);
            switch (selResizerNo) {
            case RESIZER_TOP_LEFT:      tempScrRect.setTopLeft(tempMousePos);        break;
            case RESIZER_TOP_RIGHT:     tempScrRect.setTopRight(tempMousePos);       break;
            case RESIZER_BOTTOM_LEFT:   tempScrRect.setBottomLeft(tempMousePos);     break;
            default:                    tempScrRect.setBottomRight(tempMousePos);    break;
            }
            // Update resized screen obj to yolo style rect
            float x = (tempScrRect.left() - rectDraw.left()) / rectDraw.width();
            float y = (tempScrRect.top() - rectDraw.top()) / rectDraw.height();
            float w = tempScrRect.width() / rectDraw.width();
            float h = tempScrRect.height() / rectDraw.height();
            rectYoloObjs.at(selObjNo)->setRect(x, y, w, h);
            rectYoloObjs.at(selObjNo)->setSize(QSizeF(w, h));

            cursonOn = true;
        }
        else {
            // determine that any resizer is selected.
            for (int objNo = 0; objNo < rectResizers.count(); objNo++) {
                for (int modNo = 0; modNo < RESIZER_TOTAL; modNo++) {
                    if (rectResizers.at(objNo).at(modNo)->marginsAdded(QMarginsF(5, 5, 5, 5)).contains(tempMousePos)) {
                        selObjNo = objNo;
                        selResizerNo = modNo;

                        // Select mouse cursor
                        QCursor cursor;
                        switch (modNo) {
                        case RESIZER_TOP_LEFT:
                        case RESIZE_BOTTOM_RIGHT:
                            cursor.setShape(Qt::SizeFDiagCursor);
                            break;
                        case RESIZER_TOP_RIGHT:
                        case RESIZER_BOTTOM_LEFT:
                            cursor.setShape(Qt::SizeBDiagCursor);
                            break;
                        }
                        setCursor(cursor);
                        cursonOn = true;
                        break;
                    }
                }
            }
        }

        // Just move mouse
        if (!dragImg) {
            mousePos = tempMousePos;
        }

        // Curson is not on resizer
        if (!dragResizer && !cursonOn) {
            selObjNo = selResizerNo = -1;
        }

        // Reset mouse cursor
        if (!dragImg && !dragResizer && !cursonOn) {
            // Set cross cursor
            auto cursor = QCursor(Qt::CrossCursor);
            setCursor(cursor);
        }

        update();
    }
    return QWidget::eventFilter(watched, event);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (!img.isNull()) {
        // Drag image
        if (event->button() == Qt::RightButton) {
            dragStart = event->position();
            dragImg = true;

            // Set closed hand cursor
            auto cursor = QCursor(Qt::ClosedHandCursor);
            setCursor(cursor);
        }
        // Select object's resizer
        else if (event->button() == Qt::LeftButton) {
            if (selResizerNo != -1) {
                dragResizer = true;
            }
        }
        // Reset zoom
        else if (event->button() == Qt::MiddleButton) {
            zoomRatio = 1.0;
            rectDraw = rectOriginalDraw;
        }
        else {}

        update();
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    // Set open hand cursor
    if (dragImg) {
        auto cursor = QCursor(Qt::OpenHandCursor);
        setCursor(cursor);
    }

    if (newObj) {
    }

    dragImg = dragResizer = newObj = false;

    update();
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !img.isNull()) {
        newObj = true;
        posNewObj = event->position();
    }
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    // Zoom in
    if (event->angleDelta().y() > 0) {
        zoomRatio += 0.1;
        if (zoomRatio > 5) {
            zoomRatio = 5;
        }
    }
    // Zoom out
    else {
        zoomRatio -= 0.1;
        if (zoomRatio < 1) {
            zoomRatio = 1;
        }
    }

    // update target rect size and position
    auto preSize = rectDraw.size();
    rectDraw.setSize(rectOriginalDraw.size() * zoomRatio);
    auto deltaSize = (rectDraw.size() - preSize) / 2.0f;
    rectDraw.moveTopLeft(rectDraw.topLeft() - QPointF(deltaSize.width(), deltaSize.height()));
    moveDrawRectCenter(rectDraw.center());

    update();
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Create painter
    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw Image
    p.fillRect(rect(), QBrush(Qt::darkGray, Qt::Dense5Pattern));
    p.drawImage(rectDraw, img);

    // Draw mouse pos
    if (!dragImg) {
        p.setPen(Qt::white);
        p.drawLine(mousePos.x(), 0, mousePos.x(), height());
        p.drawLine(0, mousePos.y(), width(), mousePos.y());
    }

    // Draw objects
    updateObjectScreenRect();
    for (int objNo = 0; objNo < rectYoloObjs.count(); objNo++) {
        QColor colorPen = classColors.at(objClass.at(objNo));

        // Draw object rects
        QColor colorBrush(colorPen);
        colorBrush.setAlphaF(0.2f);
        p.setBrush(objNo == selObjNo ? Qt::transparent : colorBrush);
        p.setPen(QPen(colorPen, 2, objNo == selObjNo ? Qt::DotLine : Qt::SolidLine));
        p.drawRect(*rectScrObjs.at(objNo));

        // Draw object's size modifier
        p.setBrush(colorPen);
        p.setPen(QPen(Qt::transparent));
        for (int modNo = 0; modNo < rectResizers.at(objNo).count(); modNo++) {
            auto mod = rectResizers.at(objNo).at(modNo);
            p.drawEllipse((objNo == selObjNo && modNo == selResizerNo) ? mod->marginsAdded(QMarginsF(2, 2, 2, 2)) : *mod);
        }
    }

    p.end();
}
