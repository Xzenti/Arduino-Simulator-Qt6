#ifndef ARDUINOBOARDITEM_H
#define ARDUINOBOARDITEM_H

#include <QGraphicsItem>
#include <QPixmap>

class ArduinoBoardItem : public QGraphicsItem {
public:
    ArduinoBoardItem(QGraphicsItem *parent = nullptr);
    ~ArduinoBoardItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPixmap boardPixmap;
    static constexpr int MaxDisplayWidth = 560;
    static constexpr int MaxDisplayHeight = 420;
};

#endif // ARDUINOBOARDITEM_H
