#ifndef BUTTONITEM_H
#define BUTTONITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPointF>
#include <QString>

class ButtonItem : public QGraphicsItem {
public:
    explicit ButtonItem(QGraphicsItem *parent = nullptr);
    ~ButtonItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setPressed(bool pressed);
    bool isPressed() const { return pressed; }

    void setSimulationMode(bool active);
    bool isSimulationMode() const { return simulationMode_; }

    // Pin positions in item-local coords (50×50). Use mapToScene(pin1Pos()) etc. for wiring
    QPointF pin1Pos() const;  // top-left
    QPointF pin2Pos() const;  // top-right
    QPointF pin3Pos() const;  // bottom-left
    QPointF pin4Pos() const;  // bottom-right

    // Pin ids for wiring (pin1–pin4 to board)
    QString pin1PinId() const { return pin1PinId_; }
    void setPin1PinId(const QString &id);
    QString pin2PinId() const { return pin2PinId_; }
    void setPin2PinId(const QString &id);
    QString pin3PinId() const { return pin3PinId_; }
    void setPin3PinId(const QString &id);
    QString pin4PinId() const { return pin4PinId_; }
    void setPin4PinId(const QString &id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    void loadPixmaps();
    QString pin1PinId_;
    QString signalPinLabel() const;
    QString pin2PinId_;
    QString pin3PinId_;
    QString pin4PinId_;
    QPixmap pixmapNormal;
    QPixmap pixmapPressed;
    bool pressed = false;
    bool simulationMode_ = false;
    int pressCount_ = 0;
    static constexpr int MaxWidth = 50;
    static constexpr int MaxHeight = 50;
};

#endif // BUTTONITEM_H
