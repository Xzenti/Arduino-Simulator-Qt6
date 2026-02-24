#ifndef COMPONENTWIRE_H
#define COMPONENTWIRE_H

#include <QColor>
#include <QGraphicsItem>
#include <QPointF>

// Draws a wire between two component terminals (e.g. LED anode to Button pin1).

class ComponentWire : public QGraphicsItem {
public:

    //Creates a wire from one component terminal to another.

    ComponentWire(QGraphicsItem *fromItem, int fromTerminalIndex,
                  QGraphicsItem *toItem, int toTerminalIndex,
                  QGraphicsItem *parent = nullptr);
    ~ComponentWire();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    QGraphicsItem *fromItem() const { return fromItem_; }
    int fromTerminalIndex() const { return fromTerminalIndex_; }
    QGraphicsItem *toItem() const { return toItem_; }
    int toTerminalIndex() const { return toTerminalIndex_; }

    // Call when either component moves so the wire path updates
    void updateEndpoints();

    // Wire color matching board connection (green/gray for LED, orange resistor, blue button)
    QColor wireColor() const;

    // Returns the scene position of a component terminal

    static QPointF getTerminalScenePosition(QGraphicsItem *item, int terminalIndex);

private:
    QGraphicsItem *fromItem_ = nullptr;
    int fromTerminalIndex_ = 0;
    QGraphicsItem *toItem_ = nullptr;
    int toTerminalIndex_ = 0;
    QPointF fromScene_;
    QPointF toScene_;
};

#endif // COMPONENTWIREITEM_H
