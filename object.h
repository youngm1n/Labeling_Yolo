#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QComboBox>
#include <QPointF>
#include <QSizeF>

class object;
typedef QList<object *> OBJECTS;

class object : public QObject
{
    Q_OBJECT
public:
    explicit object(int classNo, float &posX, float &posY, float &width, float &height, QObject *parent = nullptr);

    int getClassNo();
    float getPosX();
    float getPosY();
    float getWidth();
    float getHeight();

    QPointF getPos();
    QSizeF getSize();
    QRectF getRect();

    void setClassNo(int newClassNo);
    void setPosX(float newPosX);
    void setPosY(float newPosY);
    void setWidth(float newWidth);
    void setHeight(float newHeight);

    void setRect(float newPosX, float newPosY, float newWidth, float newHeight);

private:
    int classNo;

    float posX;
    float posY;
    float width;
    float height;

signals:

};

#endif // OBJECT_H
