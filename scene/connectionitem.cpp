#include "ConnectionItem.h"
#include "ArduinoBoardItem.h"
#include "LedItem.h"
#include "ResistorItem.h"
#include "ButtonItem.h"
#include "PinPositions.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QToolTip>
#include <QGraphicsSceneHoverEvent>

static QString connectionTooltip(const QString &pinId, LedItem *led, bool toAnode,
                                 ResistorItem *res, int resLeg, ButtonItem *btn, int btnPin) {
    QString from = pinId.isEmpty() ? QString("Pin") : QString("Pin%1").arg(pinId);
    if (led)
        return QString("%1 → LED:%2").arg(from).arg(toAnode ? "A" : "C");
    if (res)
        return QString("%1 → R:L%2").arg(from).arg(resLeg);
    if (btn)
        return QString("%1 → Btn:P%2").arg(from).arg(btnPin);
    return from;
}

ConnectionItem::ConnectionItem(ArduinoBoardItem *board, const QString &pinId, LedItem *led, bool toAnode, QGraphicsItem *parent)
    : QGraphicsItem(parent), board_(board), pinId_(pinId), led_(led), toAnode_(toAnode) {
    setZValue(1);
    setFlag(ItemStacksBehindParent, false);
    setAcceptHoverEvents(true);
    updateEndpoints();
    setToolTip(connectionTooltip(pinId_, led_, toAnode_, nullptr, 0, nullptr, 0));
}

ConnectionItem::ConnectionItem(ArduinoBoardItem *board, const QString &pinId, ResistorItem *res, int leg, QGraphicsItem *parent)
    : QGraphicsItem(parent), board_(board), pinId_(pinId), res_(res), resLeg_(leg) {
    setZValue(1);
    setFlag(ItemStacksBehindParent, false);
    setAcceptHoverEvents(true);
    updateEndpoints();
    setToolTip(connectionTooltip(pinId_, nullptr, false, res_, resLeg_, nullptr, 0));}

ConnectionItem::ConnectionItem(ArduinoBoardItem *board, const QString &pinId, ButtonItem *btn, int pinNum, QGraphicsItem *parent)
    : QGraphicsItem(parent), board_(board), pinId_(pinId), btn_(btn), btnPin_(pinNum) {
    setZValue(1);
    setFlag(ItemStacksBehindParent, false);
    setAcceptHoverEvents(true);
    updateEndpoints();
    setToolTip(connectionTooltip(pinId_, nullptr, false, res_, resLeg_, nullptr, 0));
}

ConnectionItem::~ConnectionItem() = default;

void ConnectionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event);
    QToolTip::hideText();
    QGraphicsItem::hoverLeaveEvent(event);
}

// Off-board routing for A0–A5 / power: wire runs just beside the board
static constexpr qreal OFF_BOARD_EXIT_OFFSET = -40;
// Max horizontal distance the wire runs beside the board before going up to the component
static constexpr qreal OFF_BOARD_MAX_RUN = 10;

QRectF ConnectionItem::boundingRect() const {
    qreal fx = fromScene_.x(), fy = fromScene_.y();
    qreal tx = toScene_.x(), ty = toScene_.y();
    qreal midY = (fy + ty) * 0.5;
    qreal midX = (fx + tx) * 0.5;
    qreal minX = qMin(qMin(fx, tx), midX);
    qreal maxX = qMax(qMax(fx, tx), midX);
    qreal minY = qMin(qMin(fy, ty), midY);
    qreal maxY = qMax(qMax(fy, ty), midY);
    if (board_ && PinPositions::isBottomRowPin(pinId_)) {
        QRectF br = board_->sceneBoundingRect();
        qreal belowY = br.bottom() + OFF_BOARD_EXIT_OFFSET;
        qreal runoffX;
        if (wireGoesToLeftSide())
            runoffX = qMax(br.left() - OFF_BOARD_MAX_RUN, qMin(tx, br.left() - OFF_BOARD_EXIT_OFFSET));
        else
            runoffX = qMin(br.right() + OFF_BOARD_MAX_RUN, qMax(tx, br.right() + OFF_BOARD_EXIT_OFFSET));
        minX = qMin(minX, runoffX);
        maxX = qMax(maxX, runoffX);
        minY = qMin(minY, belowY);
        maxY = qMax(maxY, belowY);
    }
    qreal margin = 4;
    return QRectF(minX - margin, minY - margin, maxX - minX + 2 * margin, maxY - minY + 2 * margin);
}

QPainterPath ConnectionItem::shape() const {
    if (!board_ || pinId_.isEmpty() || (!led_ && !res_ && !btn_))
        return QPainterPath();
    qreal fx = fromScene_.x(), fy = fromScene_.y();
    qreal tx = toScene_.x(), ty = toScene_.y();
    qreal midY = (fy + ty) * 0.5;
    qreal midX = (fx + tx) * 0.5;
    QPolygonF path;
    if (board_ && PinPositions::isBottomRowPin(pinId_)) {
        QRectF boardRect = board_->sceneBoundingRect();
        qreal turnY = boardRect.bottom() + OFF_BOARD_EXIT_OFFSET;
        qreal runoffX;
        if (wireGoesToLeftSide()) {
            runoffX = qMax(boardRect.left() - OFF_BOARD_MAX_RUN,
                           qMin(tx, boardRect.left() - OFF_BOARD_EXIT_OFFSET));
        } else {
            runoffX = qMin(boardRect.right() + OFF_BOARD_MAX_RUN,
                           qMax(tx, boardRect.right() + OFF_BOARD_EXIT_OFFSET));
        }
        path << fromScene_
             << QPointF(fx, turnY)
             << QPointF(runoffX, turnY)
             << QPointF(runoffX, ty)
             << toScene_;
    } else if (btn_) {
        path << toScene_ << QPointF(midX, ty) << QPointF(midX, midY) << QPointF(fx, midY) << fromScene_;
    } else if (res_) {
        path << toScene_ << QPointF(fx, ty) << fromScene_;
    } else {
        path << fromScene_ << QPointF(fx, midY) << QPointF(tx, midY) << toScene_;
    }
    if (path.isEmpty())
        return QPainterPath();
    QPainterPath linePath;
    linePath.moveTo(path.first());
    for (int i = 1; i < path.size(); ++i)
        linePath.lineTo(path.at(i));
    QPainterPathStroker stroker;
    stroker.setWidth(12);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(linePath);
}


void ConnectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (!board_ || pinId_.isEmpty() || (!led_ && !res_ && !btn_))
        return;
    updateEndpoints();
    QColor wireColor;
    if (led_)
        wireColor = toAnode_ ? QColor(0, 160, 0) : QColor(60, 60, 60);
    else if (res_)
        wireColor = QColor(200, 120, 0);  // orange for resistor
    else
        wireColor = QColor(100, 149, 237);  // cornflower blue for button
    painter->setPen(QPen(wireColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    qreal fx = fromScene_.x(), fy = fromScene_.y();
    qreal tx = toScene_.x(), ty = toScene_.y();
    qreal midY = (fy + ty) * 0.5;
    qreal midX = (fx + tx) * 0.5;
    QPolygonF path;

    if (board_ && PinPositions::isBottomRowPin(pinId_)) {

        // Side (left/right) is fixed by terminal
        QRectF boardRect = board_->sceneBoundingRect();
        qreal turnY = boardRect.bottom() + OFF_BOARD_EXIT_OFFSET;
        qreal runoffX;
        if (wireGoesToLeftSide()) {
            runoffX = qMax(boardRect.left() - OFF_BOARD_MAX_RUN,
                           qMin(tx, boardRect.left() - OFF_BOARD_EXIT_OFFSET));
        } else {
            runoffX = qMin(boardRect.right() + OFF_BOARD_MAX_RUN,
                           qMax(tx, boardRect.right() + OFF_BOARD_EXIT_OFFSET));
        }
        // Path: pin → vertical → horizontal (beside board) → vertical → horizontal → component
        path << fromScene_
             << QPointF(fx, turnY)
             << QPointF(runoffX, turnY)
             << QPointF(runoffX, ty)
             << toScene_;
    } else if (btn_) {
        // Button path (D0–D13 / GND / AREF / SDA / SCL): button → horizontal → vertical → horizontal → vertical → pin
        path << toScene_ << QPointF(midX, ty) << QPointF(midX, midY) << QPointF(fx, midY) << fromScene_;
    } else if (res_) {
        // Resistor path: resistor → horizontal → vertical → pin
        path << toScene_ << QPointF(fx, ty) << fromScene_;
    } else {
        // LED path: pin → vertical → horizontal → component
        path << fromScene_ << QPointF(fx, midY) << QPointF(tx, midY) << toScene_;
    }
    painter->drawPolyline(path);
}

bool ConnectionItem::wireGoesToLeftSide() const {
    // Fix wire side by terminal/leg so it does not flip when the component is dragged.
    if (btn_)
        return (btnPin_ == 1 || btnPin_ == 3);  // pin1/pin3 = left side of button
    if (res_)
        return (resLeg_ == 1);                  // leg1 = left side of resistor
    if (led_)
        return !toAnode_;                        // cathode = left, anode = right
    return true;
}

void ConnectionItem::updateEndpoints() {
    if (!board_)
        return;
    if (led_) {
        prepareGeometryChange();
        QPointF pinLocal = PinPositions::getBoardPinPosition(pinId_);
        fromScene_ = board_->mapToScene(pinLocal);
        toScene_ = toAnode_ ? led_->mapToScene(led_->anodePos()) : led_->mapToScene(led_->cathodePos());
    } else if (res_) {
        prepareGeometryChange();
        QPointF pinLocal = PinPositions::getBoardPinPosition(pinId_);
        fromScene_ = board_->mapToScene(pinLocal);
        toScene_ = (resLeg_ == 1) ? res_->mapToScene(res_->leg1Pos()) : res_->mapToScene(res_->leg2Pos());
    } else if (btn_) {
        prepareGeometryChange();
        QPointF pinLocal = PinPositions::getBoardPinPosition(pinId_);
        fromScene_ = board_->mapToScene(pinLocal);
        QPointF toLocal;
        switch (btnPin_) {
        case 1: toLocal = btn_->pin1Pos(); break;
        case 2: toLocal = btn_->pin2Pos(); break;
        case 3: toLocal = btn_->pin3Pos(); break;
        case 4: toLocal = btn_->pin4Pos(); break;
        default: toLocal = btn_->pin1Pos(); break;
        }
        toScene_ = btn_->mapToScene(toLocal);
    }
}
