#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QComboBox>
#include <QPointF>
#include <QSizeF>

enum ENUM_RECT_CORNER  { RECT_TOP_LEFT, RECT_TOP_RIGHT, RECT_BOTTOM_LEFT, RECT_BOTTOM_RIGHT, RECT_CORNER_TOTAL };

class object;
typedef QList<object *> OBJECTS;
typedef QList<QRectF *> RESIZERS;

class object : public QObject
{
    Q_OBJECT
public:
    explicit object(int classNo, QString className, QColor classColor, float yoloCenterX, float yoloCenterY, float yoloWidth, float yoloHeight, QObject *parent = nullptr);
    explicit object(int classNo, QString className, QColor classColor, QRectF scrRect, QRectF rectDraw, QObject *parent = nullptr);

    void init(int classNo, QString className, QColor classColor);

    void updateYoloRect(float yoloCenterX, float yoloCenterY, float yoloWidth, float yoloHeight);
    void updateYoloRect(const QRectF &scrRect, const QRectF &rectDraw);

    void setClassNo(int newClassNo);
    int getClassNo() const;
    QString getClassName() const;
    QColor getClassColor() const;

    QRectF getYoloRect();
    QRectF getScrRect(const QRectF &rectDraw);
    RESIZERS getScrResizers() const;

    int getSelectedResizerNo(const QPointF &pos);

private:
    int classNo;
    QString className;
    QColor classColor;
    
    QRectF rectYolo;
    QRectF rectScr;
    RESIZERS scrResizers;

signals:

};

#endif // OBJECT_H
