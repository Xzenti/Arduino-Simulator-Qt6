#include "ButtonItem.h"
#include "PinPositions.h"
#include "../core/Logger.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QCursor>
ButtonItem::ButtonItem(QGraphicsItem *parent)
    : QGraphicsItem(parent) {
    loadPixmaps();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
}

ButtonItem::~ButtonItem() = default;

void ButtonItem::loadPixmaps() {
    pixmapNormal = QPixmap(":/images/button.png");
    pixmapPressed = QPixmap(":/images/button_pressed.png");
    if (pixmapNormal.isNull() || pixmapPressed.isNull()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList bases;
        bases << (appDir + "/../resources/images/");
        bases << (appDir + "/../../resources/images/");
        bases << "resources/images/";
        for (const QString &base : bases) {
            QString absBase = QFileInfo(base).absoluteFilePath();
            if (QFileInfo(absBase).isDir()) {
                if (pixmapNormal.isNull())
                    pixmapNormal.load(absBase + "/button.png");
                if (pixmapPressed.isNull())
                    pixmapPressed.load(absBase + "/button_pressed.png");
                if (!pixmapNormal.isNull() && !pixmapPressed.isNull())
                    break;
            }
        }
    }
    if (pixmapNormal.isNull()) {
        pixmapNormal = QPixmap(MaxWidth, MaxHeight);
        pixmapNormal.fill(Qt::gray);
    }
    if (pixmapPressed.isNull()) {
        pixmapPressed = QPixmap(MaxWidth, MaxHeight);
        pixmapPressed.fill(Qt::darkGray);
    }
    pixmapNormal = pixmapNormal.scaled(PinPositions::BUTTON_WIDTH, PinPositions::BUTTON_HEIGHT,
                                       Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pixmapPressed = pixmapPressed.scaled(PinPositions::BUTTON_WIDTH, PinPositions::BUTTON_HEIGHT,
                                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QRectF ButtonItem::boundingRect() const {
    return QRectF(0, 0, PinPositions::BUTTON_WIDTH, PinPositions::BUTTON_HEIGHT);
}

QPointF ButtonItem::pin1Pos() const {
    return QPointF(PinPositions::BUTTON_PIN1_X, PinPositions::BUTTON_PIN1_Y);
}

QPointF ButtonItem::pin2Pos() const {
    return QPointF(PinPositions::BUTTON_PIN2_X, PinPositions::BUTTON_PIN2_Y);
}

QPointF ButtonItem::pin3Pos() const {
    return QPointF(PinPositions::BUTTON_PIN3_X, PinPositions::BUTTON_PIN3_Y);
}

QPointF ButtonItem::pin4Pos() const {
    return QPointF(PinPositions::BUTTON_PIN4_X, PinPositions::BUTTON_PIN4_Y);
}

void ButtonItem::setPin1PinId(const QString &id) {
    if (pin1PinId_ != id) { pin1PinId_ = id; update(); }
}

void ButtonItem::setPin2PinId(const QString &id) {
    if (pin2PinId_ != id) { pin2PinId_ = id; update(); }
}

void ButtonItem::setPin3PinId(const QString &id) {
    if (pin3PinId_ != id) { pin3PinId_ = id; update(); }
}

void ButtonItem::setPin4PinId(const QString &id) {
    if (pin4PinId_ != id) { pin4PinId_ = id; update(); }
}

void ButtonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const QPixmap &pm = pressed ? pixmapPressed : pixmapNormal;
    painter->drawPixmap(0, 0, pm);
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}
QString ButtonItem::signalPinLabel() const {
    // Check all four pin slots; return the first one that is a digital/analog signal pin
    // (skip GND, 5V, 3.3V — those are power, not the signal the sketch reads)
    static const QStringList powerPins = { "GND", "5V", "3.3V", "VIN", "RESET", "IOREF", "AREF" };
    const QString pins[4] = { pin1PinId_, pin2PinId_, pin3PinId_, pin4PinId_ };
    for (const QString &id : pins) {
        if (!id.isEmpty() && !powerPins.contains(id, Qt::CaseInsensitive))
            return id;  // e.g. "D2", "D3", "A0"
    }
    // Fallback: return any connected pin, or "unassigned"
    for (const QString &id : pins) {
        if (!id.isEmpty())
            return id;
    }
    return "unassigned";
}

void ButtonItem::setPressed(bool p) {
    if (pressed != p) {
        pressed = p;
        if (simulationMode_ && p) {
            ++pressCount_;
            // Log only the first press
            if (pressCount_ == 1) {
                Logger::instance().info(
                    QString("Button (pin %1) activated — input pulled LOW")
                        .arg(signalPinLabel()));
            }
        }
        update();
    }
}

void ButtonItem::setSimulationMode(bool active) {
    if (simulationMode_ == active)
        return;
    simulationMode_ = active;

    if (active) {
        // Entering simulation: reset interaction counter, lock position
        pressCount_ = 0;
        setFlag(ItemIsMovable, false);
        setFlag(ItemIsSelectable, false);
        setCursor(Qt::PointingHandCursor);
    } else {
        // Leaving simulation: log summary if button was used, restore design mode
        if (pressCount_ > 0) {
            Logger::instance().info(
                QString("Button (pin %1): %2 press%3 during simulation")
                    .arg(signalPinLabel())
                    .arg(pressCount_)
                    .arg(pressCount_ == 1 ? "" : "es"));
        }
        pressCount_ = 0;
        setFlag(ItemIsMovable, true);
        setFlag(ItemIsSelectable, true);
        unsetCursor();
        // Ensure button is released when simulation stops
        if (pressed) {
            pressed = false;
            update();
        }
    }
}

void ButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event);
    if (simulationMode_)
        setCursor(Qt::PointingHandCursor);
}

void ButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event);
    if (simulationMode_)
        setCursor(Qt::PointingHandCursor);  // keep hand while in sim mode
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setPressed(true);
    QGraphicsItem::mousePressEvent(event);
}

void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setPressed(false);
    QGraphicsItem::mouseReleaseEvent(event);
}
