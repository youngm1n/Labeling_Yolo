#include "object.h"

object::object(int newClassNo, float &newPosX, float &newPosY, float &newWidth, float &newHeight, QObject *parent)
    : QObject{parent}
{
    classNo = newClassNo;
    posX = newPosX;
    posY = newPosY;
    width = newWidth;
    height = newHeight;
}

int object::getClassNo()
{
    return classNo;
}

float object::getPosX()
{
    return posX;
}

float object::getPosY()
{
    return posY;
}

float object::getWidth()
{
    return width;
}

float object::getHeight()
{
    return height;
}

QPointF object::getPos()
{
    return QPointF(posX, posY);
}

QSizeF object::getSize()
{
    return QSize(width, height);
}

QRectF object::getRect()
{
    return QRectF(posX, posY, width, height);
}

void object::setClassNo(int newClassNo)
{
    classNo = newClassNo;
}

void object::setPosX(float newPosX)
{
    posX = newPosX;
}

void object::setPosY(float newPosY)
{
    posY = newPosY;
}

void object::setWidth(float newWidth)
{
    width = newWidth;
}

void object::setHeight(float newHeight)
{
    height = newHeight;
}

void object::setRect(float newPosX, float newPosY, float newWidth, float newHeight)
{
    posX = newPosX;
    posY = newPosY;
    width = newWidth;
    height = newHeight;
}
