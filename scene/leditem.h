#ifndef LEDITEM_H
#define LEDITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPointF>
#include <QString>

class LedItem : public QGraphicsItem {
public:
    LedItem(int pin = -1, QGraphicsItem *parent = nullptr);
    ~LedItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setState(bool on);
    bool isOn() const { return isOn_; }
    // Digital pin number 0-13 for simulation (from pinId); -1 if not a digital pin
    int getPin() const;
    void setPin(int p);

    // Anode pin id (e.g. "D13")
    QString pinId() const { return pinId_; }
    void setPinId(const QString &id);
    // Cathode pin id (e.g. "GND")
    QString cathodePinId() const { return cathodePinId_; }
    void setCathodePinId(const QString &id);

    // Anode leg position in item-local coords (48×48). Use mapToScene(anodePos()) for wiring
    QPointF anodePos() const;
    // Cathode leg position in item-local coords (48×48). Use mapToScene(cathodePos()) for wiring
    QPointF cathodePos() const;

private:
    void loadPixmaps();
    QString pinId_;       // anode pin (e.g. D13)
    QString cathodePinId_; // cathode pin (e.g. GND)
    int pin = -1;
    bool isOn_ = false;
    QPixmap pixmapOn;
    QPixmap pixmapOff;
    static constexpr int MaxSize = 48;
};

#endif // LEDITEM_H
