#ifndef CONNECTIONITEM_H
#define CONNECTIONITEM_H

#include <QGraphicsItem>
#include <QString>

class ArduinoBoardItem;
class LedItem;
class ResistorItem;
class ButtonItem;

// Draws a wire from an Arduino pin to LED, resistor, or button pin.
class ConnectionItem : public QGraphicsItem {
public:
    // Wire to LED: toAnode true = anode, false = cathode
    ConnectionItem(ArduinoBoardItem *board, const QString &pinId, LedItem *led, bool toAnode, QGraphicsItem *parent = nullptr);
    // Wire to resistor: leg 1 = leg1, leg 2 = leg2
    ConnectionItem(ArduinoBoardItem *board, const QString &pinId, ResistorItem *res, int leg, QGraphicsItem *parent = nullptr);
    // Wire to button: pinNum 1–4 = pin1 (top-left) … pin4 (bottom-right)
    ConnectionItem(ArduinoBoardItem *board, const QString &pinId, ButtonItem *btn, int pinNum, QGraphicsItem *parent = nullptr);
    ~ConnectionItem();

    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    ArduinoBoardItem *boardItem() const { return board_; }
    QString pinId() const { return pinId_; }
    LedItem *ledItem() const { return led_; }
    ResistorItem *resistorItem() const { return res_; }
    ButtonItem *buttonItem() const { return btn_; }
    bool isToAnode() const { return toAnode_; }
    int resistorLeg() const { return resLeg_; }
    int buttonPin() const { return btnPin_; }

    // Call when board or component moved so line updates
    void updateEndpoints();

    bool wireGoesToLeftSide() const;

private:
    ArduinoBoardItem *board_ = nullptr;
    QString pinId_;
    LedItem *led_ = nullptr;
    ResistorItem *res_ = nullptr;
    ButtonItem *btn_ = nullptr;
    bool toAnode_ = true;
    int resLeg_ = 1;
    int btnPin_ = 1;
    QPointF fromScene_;
    QPointF toScene_;
};

#endif // CONNECTIONITEM_H
