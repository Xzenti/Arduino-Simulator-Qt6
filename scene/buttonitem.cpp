#include "ButtonItem.h"
#include "PinPositions.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QFileInfo>

ButtonItem::ButtonItem(QGraphicsItem *parent)
    : QGraphicsItem(parent) {
    loadPixmaps();
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

void ButtonItem::setPressed(bool p) {
    if (pressed != p) {
        pressed = p;
        update();
    }
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    setPressed(true);
    QGraphicsItem::mousePressEvent(event);
}

void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setPressed(false);
    QGraphicsItem::mouseReleaseEvent(event);
}
