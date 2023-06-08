#include "object.h"

object::object(int newClassNo, float &newCenterX, float &newCenterY, float &newWidth, float &newHeight, QObject *parent)
    : QObject{parent}
{
    classNo = newClassNo;
    centerX = newCenterX;
    centerY = newCenterY;
    width = newWidth;
    height = newHeight;
}

int object::getClassNo()
{
    return classNo;
}

float object::getCenterX()
{
    return centerX;
}

float object::getCenterY()
{
    return centerY;
}

float object::getWidth()
{
    return width;
}

float object::getHeight()
{
    return height;
}

QPointF object::getCenterPos()
{
    return QPointF(centerX, centerY);
}

QSizeF object::getSize()
{
    return QSize(width, height);
}

QRectF object::getRect()
{
    return QRectF(centerX - width / 2.0f, centerY - height / 2.0f, width, height);
}

void object::setClassNo(int newClassNo)
{
    classNo = newClassNo;
}

void object::setCenterX(float newCenterX)
{
    centerX = newCenterX;
}

void object::setCenterY(float newCenterY)
{
    centerY = newCenterY;
}

void object::setWidth(float newWidth)
{
    width = newWidth;
}

void object::setHeight(float newHeight)
{
    height = newHeight;
}

