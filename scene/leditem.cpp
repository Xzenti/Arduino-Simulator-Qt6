#include "LedItem.h"
#include "PinPositions.h"
#include <QPainter>
#include <QCoreApplication>
#include <QFileInfo>

LedItem::LedItem(int pin, QGraphicsItem *parent)
    : QGraphicsItem(parent), pin(pin) {
    if (pin >= 0 && pin <= 13)
        pinId_ = QString("D%1").arg(pin);
    loadPixmaps();
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
}

LedItem::~LedItem() = default;

void LedItem::loadPixmaps() {
    pixmapOn = QPixmap(":/images/led_on.png");
    pixmapOff = QPixmap(":/images/led_off.png");
    if (pixmapOn.isNull() || pixmapOff.isNull()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList bases;
        bases << (appDir + "/../resources/images/");
        bases << (appDir + "/../../resources/images/");
        bases << "resources/images/";
        for (const QString &base : bases) {
            QString absBase = QFileInfo(base).absoluteFilePath();
            if (QFileInfo(absBase).isDir()) {
                if (pixmapOn.isNull())
                    pixmapOn.load(absBase + "/led_on.png");
                if (pixmapOff.isNull())
                    pixmapOff.load(absBase + "/led_off.png");
                if (!pixmapOn.isNull() && !pixmapOff.isNull())
                    break;
            }
        }
    }
    if (pixmapOn.isNull()) {
        pixmapOn = QPixmap(MaxSize, MaxSize);
        pixmapOn.fill(Qt::red);
    }
    if (pixmapOff.isNull()) {
        pixmapOff = QPixmap(MaxSize, MaxSize);
        pixmapOff.fill(Qt::gray);
    }
    if (pixmapOn.width() > MaxSize || pixmapOn.height() > MaxSize)
        pixmapOn = pixmapOn.scaled(MaxSize, MaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (pixmapOff.width() > MaxSize || pixmapOff.height() > MaxSize)
        pixmapOff = pixmapOff.scaled(MaxSize, MaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QRectF LedItem::boundingRect() const {
    qreal w = qMax(pixmapOn.width(), pixmapOff.width());
    qreal h = qMax(pixmapOn.height(), pixmapOff.height());
    return QRectF(0, 0, w, h);
}

void LedItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const QPixmap &pm = isOn_ ? pixmapOn : pixmapOff;
    painter->drawPixmap(0, 0, pm);
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void LedItem::setState(bool on) {
    if (isOn_ != on) {
        isOn_ = on;
        update();
    }
}

QPointF LedItem::anodePos() const {
    return QPointF(PinPositions::LED_ANODE_X, PinPositions::LED_ANODE_Y);
}

QPointF LedItem::cathodePos() const {
    return QPointF(PinPositions::LED_CATHODE_X, PinPositions::LED_CATHODE_Y);
}

int LedItem::getPin() const {
    return PinPositions::pinIdToDigitalNumber(pinId_);
}

void LedItem::setPin(int p) {
    pin = p;
    if (p >= 0 && p <= 13)
        pinId_ = QString("D%1").arg(p);
}

void LedItem::setPinId(const QString &id) {
    if (pinId_ != id) {
        pinId_ = id;
        pin = PinPositions::pinIdToDigitalNumber(id);
        update();
    }
}

void LedItem::setCathodePinId(const QString &id) {
    if (cathodePinId_ != id) {
        cathodePinId_ = id;
        update();
    }
}
