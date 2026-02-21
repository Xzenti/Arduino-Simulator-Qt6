#ifndef RESISTORITEM_H
#define RESISTORITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPointF>
#include <QString>

class ResistorItem : public QGraphicsItem {
public:
    explicit ResistorItem(QGraphicsItem *parent = nullptr);
    ~ResistorItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // Leg positions in item-local coords (60×40). Use mapToScene(leg1Pos()) for wiring
    QPointF leg1Pos() const;
    QPointF leg2Pos() const;

    // Pin id for lead1 (r1:1) and lead2 (r1:2) for wiring
    QString leg1PinId() const { return leg1PinId_; }
    void setLeg1PinId(const QString &id);
    QString leg2PinId() const { return leg2PinId_; }
    void setLeg2PinId(const QString &id);

private:
    QString leg1PinId_;
    QString leg2PinId_;
    void loadPixmap();
    QPixmap pixmap;
    static constexpr int MaxWidth = 60;
    static constexpr int MaxHeight = 40;
};

#endif // RESISTORITEM_H
