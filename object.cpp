#include "object.h"

#define RESIZER_RADIUS    7

object::object(int newClassNo, float yoloCenterX, float yoloCenterY, float yoloWidth, float yoloHeight, QObject *parent)
    : QObject{parent}
{
    init(newClassNo);
    updateYoloRect(yoloCenterX, yoloCenterY, yoloWidth, yoloHeight);
}

object::object(int newClassNo, QRectF scrRect, QRectF rectDraw, QObject *parent)
    : QObject{parent}
{
    init(newClassNo);
    updateYoloRect(scrRect, rectDraw);
}

void object::init(int newClassNo)
{
    classNo = newClassNo;

    for (int i = 0; i < RECT_CORNER_TOTAL; i++) {
        scrResizers.push_back(new QRectF(0, 0, RESIZER_RADIUS, RESIZER_RADIUS));
    }
}

void object::updateYoloRect(float yoloCenterX, float yoloCenterY, float yoloWidth, float yoloHeight)
{
    rectYolo.setSize(QSizeF(yoloWidth, yoloHeight));
    rectYolo.moveCenter(QPointF(yoloCenterX, yoloCenterY));
}

void object::updateYoloRect(const QRectF &scrRect, const QRectF &rectDraw)
{
    float x = (scrRect.left() - rectDraw.left()) / rectDraw.width();
    float y = (scrRect.top() - rectDraw.top()) / rectDraw.height();
    float w = scrRect.width() / rectDraw.width();
    float h = scrRect.height() / rectDraw.height();
    rectYolo.setRect(x, y, w, h);
}

void object::setClassNo(int newClassNo)
{
    classNo = newClassNo;
}

int object::getClassNo() const
{
    return classNo;
}

RESIZERS object::getScrResizers() const
{
    return scrResizers;
}

int object::getSelectedResizerNo(const QPointF &pos)
{
    int no = RECT_CORNER_TOTAL;

    for (int i = 0; i < scrResizers.count(); i++) {
        if (scrResizers.at(i)->contains(pos)) {
            no = i;
            break;
        }
    }

    return no;
}

QRectF object::getYoloRect()
{
    return rectYolo;
}

QRectF object::getScrRect(const QRectF &rectDraw)
{
    rectScr.setSize(QSizeF(rectYolo.width() * rectDraw.width(), rectYolo.height() * rectDraw.height()));
    rectScr.moveTopLeft(QPointF(rectYolo.left() * rectDraw.width(), rectYolo.top() * rectDraw.height()) + rectDraw.topLeft());

    scrResizers.at(RECT_TOP_LEFT)->moveCenter(rectScr.topLeft());
    scrResizers.at(RECT_TOP_RIGHT)->moveCenter(rectScr.topRight());
    scrResizers.at(RECT_BOTTOM_LEFT)->moveCenter(rectScr.bottomLeft());
    scrResizers.at(RECT_BOTTOM_RIGHT)->moveCenter(rectScr.bottomRight());

    return rectScr;
}
