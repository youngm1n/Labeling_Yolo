#include "imageviewer.h"

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget{parent}
{
    qApp->installEventFilter(this);

    zoomRatio = 1;
    dragImg = dragResizer = newObj = false;

    selObjNo = selResizerNo = -1;

    // Object's class selctor, when make a new object
    newObjClassSelctor = new QComboBox(this);
    newObjClassSelctor->hide();
    newObjClassSelctor->setFocusPolicy(Qt::StrongFocus);
    newObjClassSelctor->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    connect(newObjClassSelctor, &QComboBox::activated, this, &ImageViewer::createNewObject);
}

// Update class lsit and colors list from editor
void ImageViewer::updateClassInformation(QStringList list, CLASS_COLORS colors)
{
    objClassNames = list;
    objClassColors = colors;

    newObjClassSelctor->clear();
    auto sizeColorIcon = newObjClassSelctor->style()->pixelMetric(QStyle::PM_SmallIconSize);
    for (int i = 0; i < objClassNames.count(); i++) {
        newObjClassSelctor->addItem(objClassNames.at(i));

        QPixmap pixmap(sizeColorIcon, sizeColorIcon);
        pixmap.fill(objClassColors.at(i));
        newObjClassSelctor->setItemData(i, pixmap, Qt::DecorationRole);
    }
}

void ImageViewer::loadImage(QString imgPath, OBJECTS newObjs)
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
    objs = newObjs;
    update();
}

void ImageViewer::selectObject(int objNo)
{
    if (objNo >= 0 && objNo < objs.count()) {
        selObjNo = objNo;
        update();
    }
}

void ImageViewer::changeClassNo(int objNo, int classNo)
{
    if (objNo < objs.count()) {
        objs.at(objNo)->setClassNo(classNo);
        update();
    }
}

void ImageViewer::removeObject(int objNo)
{
    if (objNo < objs.count()) {
        objs.remove(objNo);
        update();
    }
}


QRectF *ImageViewer::getScreenRectFromYoloRect(QRectF rectYolo)
{
    auto scrRect = new QRectF(0, 0, rectYolo.width() * rectDraw.width(), rectYolo.height() * rectDraw.height());
    scrRect->moveTopLeft(QPointF(rectYolo.left() * rectDraw.width(), rectYolo.top() * rectDraw.height()) + rectDraw.topLeft());
    return scrRect;
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

void ImageViewer::createNewObject(int classNo)
{
    newObj = false;
    newObjClassSelctor->hide();

    if (rectNewObj.width() < 0) {
        auto temp = rectNewObj.left();
        rectNewObj.setLeft(rectNewObj.right());
        rectNewObj.setRight(temp);
    }
    if (rectNewObj.height() < 0) {
        auto temp = rectNewObj.top();
        rectNewObj.setTop(rectNewObj.bottom());
        rectNewObj.setBottom(temp);
    }

    auto newObj = new object(classNo, rectNewObj, rectDraw);
    objs.push_back(newObj);

    update();

    // update object for main window
    emit updateObjectsFromImageViewer(objs);
}

bool ImageViewer::eventFilter(QObject *watched, QEvent *event)
{
    // Get mouse move position
    if (watched == this && event->type() == QEvent::MouseMove) {
        bool cursorOn = false;
        auto mousePos = static_cast<QMouseEvent *>(event)->position();

        // Create new object
        if (newObj && !newObjClassSelctor->isVisible()) {
            rectNewObj = QRectF(posNewObj, mousePos);
        }
        // Drag screen
        else if (dragImg) {
            dragDelta = mousePos - dragStart;
            moveDrawRectCenter(rectDraw.center() + dragDelta);
            update();

            dragStart = mousePos;
        }
        // Resize bounding rect
        else if (dragResizer) {
            auto tempScrRect = objs.at(selObjNo)->getScrRect(rectDraw);
            switch (selResizerNo) {
            case RECT_TOP_LEFT:     tempScrRect.setTopLeft(mousePos);        break;
            case RECT_TOP_RIGHT:    tempScrRect.setTopRight(mousePos);       break;
            case RECT_BOTTOM_LEFT:  tempScrRect.setBottomLeft(mousePos);     break;
            default:                tempScrRect.setBottomRight(mousePos);    break;
            }

            // Update resized screen obj to yolo style rect
            objs.at(selObjNo)->updateYoloRect(tempScrRect, rectDraw);
            emit updateObjectsFromImageViewer(objs);
            cursorOn = true;
        }
        else {
            // determine that any resizer is selected.
            for (int objNo = 0; objNo < objs.count(); objNo++) {
                auto resizerNo = objs.at(objNo)->getSelectedResizerNo(mousePos);
                if (resizerNo < RECT_CORNER_TOTAL) {
                    selObjNo = objNo;
                    selResizerNo = resizerNo;

                    // Select mouse cursor
                    QCursor cursor;
                    switch (selResizerNo) {
                    case RECT_TOP_LEFT:
                    case RECT_BOTTOM_RIGHT:
                        cursor.setShape(Qt::SizeFDiagCursor);
                        break;
                    case RECT_TOP_RIGHT:
                    case RECT_BOTTOM_LEFT:
                        cursor.setShape(Qt::SizeBDiagCursor);
                        break;
                    }
                    setCursor(cursor);
                    cursorOn = true;

                    break;
                }
            }
        }

        // Just move mouse
        if (!dragImg) {
            currentMousePos = mousePos;
        }

        // Curson is not on resizer
        if (!dragResizer && !cursorOn) {
            selObjNo = selResizerNo = -1;
        }

        // Reset mouse cursor
        if (!dragImg && !dragResizer && !cursorOn) {
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

    if (newObj && !newObjClassSelctor->isVisible()) {
        newObjClassSelctor->move(event->position().x(), event->position().y());
        newObjClassSelctor->show();
    }

    if (dragResizer) {
        emit updateObjectsFromImageViewer(objs);
    }

    dragImg = dragResizer = false;

    update();
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Start creating new object
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
        p.drawLine(currentMousePos.x(), 0, currentMousePos.x(), height());
        p.drawLine(0, currentMousePos.y(), width(), currentMousePos.y());
    }

    // Draw objects
    if (!newObj) {
        for (int objNo = 0; objNo < objs.count(); objNo++) {
            if (selObjNo == -1 || selObjNo == objNo) {
                auto color = objClassColors.at(objs.at(objNo)->getClassNo());
                auto scrRect = objs.at(objNo)->getScrRect(rectDraw);
                auto scrResizers = objs.at(objNo)->getScrResizers();

                QColor colorBrush(color);
                colorBrush.setAlphaF(0.2f);
                p.setBrush((objNo == selObjNo && selResizerNo != -1) ? Qt::transparent : colorBrush);
                p.setPen(QPen(color, 2, objNo == selObjNo ? Qt::DotLine : Qt::SolidLine));

                // Draw bounding rect
                p.drawRect(scrRect);
                // Draw resizers
                p.setBrush(color);
                for (int i = 0; i < RECT_CORNER_TOTAL; i++) {
                    auto r = scrResizers.at(i);
                    p.drawEllipse((objNo == selObjNo && i == selResizerNo) ? r->marginsAdded(QMarginsF(2, 2, 2, 2)) : *r);
                }
            }
        }
    }
    // Draw new Object
    else {
        p.setBrush(Qt::transparent);
        p.setPen(QPen(Qt::red, 2));
        p.drawRect(rectNewObj);
    }

    p.end();
}
