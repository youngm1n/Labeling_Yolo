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
    explicit object(int classNo, float &centerX, float &centerY, float &width, float &height, QObject *parent = nullptr);

    int getClassNo();
    float getCenterX();
    float getCenterY();
    float getWidth();
    float getHeight();

    QPointF getCenterPos();
    QSizeF getSize();
    QRectF getRect();

    void setClassNo(int newClassNo);
    void setCenterX(float newPosX);
    void setCenterY(float newPosY);
    void setWidth(float newWidth);
    void setHeight(float newHeight);

private:
    int classNo;
    
    float centerX;
    float centerY;
    float width;
    float height;

signals:

};

#endif // OBJECT_H
