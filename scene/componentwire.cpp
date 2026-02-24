#include "ComponentWire.h"
#include "LedItem.h"
#include "ResistorItem.h"
#include "ButtonItem.h"
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QToolTip>
#include <QGraphicsSceneHoverEvent>

namespace {

enum class ExitDirection { VerticalDown, VerticalUp, HorizontalLeft, HorizontalRight };

ExitDirection getExitDirection(QGraphicsItem *item, int terminalIndex) {
    if (dynamic_cast<LedItem *>(item)) {

        return ExitDirection::VerticalDown;
    }
    if (ResistorItem *res = dynamic_cast<ResistorItem *>(item)) {
        Q_UNUSED(res);
        return terminalIndex == 0 ? ExitDirection::HorizontalLeft : ExitDirection::HorizontalRight;
    }
    if (ButtonItem *btn = dynamic_cast<ButtonItem *>(item)) {
        Q_UNUSED(btn);
        // pin1 (top-left), pin3 (bottom-left)
        if (terminalIndex == 0 || terminalIndex == 2)
            return ExitDirection::HorizontalLeft;
        return ExitDirection::HorizontalRight;
    }
    return ExitDirection::VerticalDown;
}

QPointF runPoint(QPointF p, ExitDirection dir, qreal run) {
    switch (dir) {
    case ExitDirection::VerticalDown:   return p + QPointF(0, run);
    case ExitDirection::VerticalUp:      return p + QPointF(0, -run);
    case ExitDirection::HorizontalLeft: return p + QPointF(-run, 0);
    case ExitDirection::HorizontalRight: return p + QPointF(run, 0);
    }
    return p;
}

QPointF approachPoint(QPointF to, ExitDirection dir, qreal run) {
    switch (dir) {
    case ExitDirection::VerticalDown:    return to + QPointF(0, -run);  // approach from above
    case ExitDirection::VerticalUp:      return to + QPointF(0, run);   // from below
    case ExitDirection::HorizontalLeft:  return to + QPointF(-run, 0);  // approach from left
    case ExitDirection::HorizontalRight: return to + QPointF(run, 0);   // approach from right
    }
    return to;
}

constexpr qreal RUN_LENGTH = 14;
constexpr qreal MIN_SEGMENT = 8;  // minimum visible length for horizontal/vertical so path doesn't collapse
constexpr qreal LED_APPROACH = 4;

void buildPath(QPointF from, QPointF to,
               QGraphicsItem *fromItem, int fromTerminalIndex,
               QGraphicsItem *toItem, int toTerminalIndex,
               QPolygonF &path) {
    ExitDirection fromDir = getExitDirection(fromItem, fromTerminalIndex);
    qreal runFrom = RUN_LENGTH;
    if (dynamic_cast<LedItem *>(fromItem) != nullptr) {

        fromDir = (to.x() >= from.x()) ? ExitDirection::HorizontalRight : ExitDirection::HorizontalLeft;
    }
    QPointF p1 = runPoint(from, fromDir, runFrom);
    bool fromButton = (dynamic_cast<ButtonItem *>(fromItem) != nullptr);
    bool fromResistor = (dynamic_cast<ResistorItem *>(fromItem) != nullptr);
    bool toLed = (dynamic_cast<LedItem *>(toItem) != nullptr);
    bool toResistor = (dynamic_cast<ResistorItem *>(toItem) != nullptr);
    bool toButton = (dynamic_cast<ButtonItem *>(toItem) != nullptr);

    if (fromResistor && toLed) {
        // Resistor → LED: horizontal → vertical → horizontal → vertical → LED
        qreal p4y = to.y() - LED_APPROACH;  // approach LED from above
        qreal runY = (from.y() < to.y()) ? to.y() + RUN_LENGTH : to.y() - 2 * RUN_LENGTH;
        if (qAbs(runY - p4y) < MIN_SEGMENT)
            runY = (from.y() < to.y()) ? p4y + RUN_LENGTH : p4y - RUN_LENGTH;
        if (qAbs(from.y() - to.y()) < MIN_SEGMENT)
            runY = (from.y() < to.y()) ? to.y() + RUN_LENGTH : to.y() - 2 * RUN_LENGTH;
        QPointF p2(p1.x(), runY);
        QPointF p3(to.x(), runY);
        QPointF p4(to.x(), p4y);
        if (qAbs(p2.x() - p3.x()) < MIN_SEGMENT) {
            qreal outX = p1.x() + (fromDir == ExitDirection::HorizontalRight ? RUN_LENGTH : -RUN_LENGTH);
            path << from << p1 << p2 << QPointF(outX, runY) << p2 << p3 << p4 << to;
        } else {
            path << from << p1 << p2 << p3 << p4 << to;
        }
    } else if (fromResistor && toButton) {
        // Resistor → Button: horizontal → vertical → horizontal → button
        ExitDirection toDir = getExitDirection(toItem, toTerminalIndex);
        QPointF p3 = approachPoint(to, toDir, RUN_LENGTH);
        qreal p2y = p3.y();
        if (qAbs(from.y() - to.y()) < MIN_SEGMENT)
            p2y = (from.y() < to.y()) ? to.y() - RUN_LENGTH : to.y() + RUN_LENGTH;
        QPointF p2(p1.x(), p2y);
        if (qAbs(p2.x() - p3.x()) < MIN_SEGMENT) {
            qreal outX = p1.x() + (fromDir == ExitDirection::HorizontalRight ? RUN_LENGTH : -RUN_LENGTH);
            path << from << p1 << p2 << QPointF(outX, p2.y()) << p2 << p3 << to;
        } else if (qAbs(p2.y() - to.y()) < MIN_SEGMENT) {
            path << from << p1 << p2 << p3 << to;
        } else {
            QPointF p2a(p1.x(), to.y());
            path << from << p1 << p2 << p2a << p3 << to;
        }
    } else if (fromButton && toLed) {
        // Button → LED: horizontal → vertical → horizontal → vertical → LED
        qreal approachY = to.y() - LED_APPROACH;
        qreal runY = (from.y() < to.y()) ? to.y() + RUN_LENGTH : to.y() - RUN_LENGTH;
        if (qAbs(runY - approachY) < MIN_SEGMENT)
            runY = (from.y() < to.y()) ? to.y() + RUN_LENGTH : to.y() - 2 * RUN_LENGTH;
        QPointF p2(p1.x(), runY);
        QPointF p3(to.x(), runY);
        QPointF p4(to.x(), approachY);
        if (qAbs(p2.x() - p3.x()) < MIN_SEGMENT) {
            qreal outX = p1.x() + RUN_LENGTH;
            path << from << p1 << p2 << QPointF(outX, runY) << p2 << p3 << p4 << to;
        } else {
            path << from << p1 << p2 << p3 << p4 << to;
        }
    } else if (fromButton && toResistor) {
        // Button → Resistor: horizontal → vertical → horizontal; ensure vertical and horizontal both visible
        ExitDirection toDir = getExitDirection(toItem, toTerminalIndex);
        QPointF p3 = approachPoint(to, toDir, RUN_LENGTH);
        qreal p2y = to.y();
        if (qAbs(from.y() - to.y()) < MIN_SEGMENT)
            p2y = (from.y() < to.y()) ? to.y() - RUN_LENGTH : to.y() + RUN_LENGTH;
        QPointF p2(p1.x(), p2y);
        if (qAbs(p2.x() - p3.x()) < MIN_SEGMENT) {
            if (p3.x() < to.x())
                p3.setX(to.x() - RUN_LENGTH - MIN_SEGMENT);
            else
                p3.setX(to.x() + RUN_LENGTH + MIN_SEGMENT);
        }
        if (qAbs(p2.y() - to.y()) < MIN_SEGMENT) {
            path << from << p1 << p2 << p3 << to;
        } else {
            QPointF p2a(p1.x(), to.y());
            path << from << p1 << p2 << p2a << p3 << to;
        }
    } else if (toResistor) {
        // LED → Resistor: vertical → horizontal → vertical
        QPointF p2(to.x(), p1.y());
        if (qAbs(p1.x() - p2.x()) < MIN_SEGMENT) {
            qreal outX = p1.x() + RUN_LENGTH;
            path << from << p1 << QPointF(outX, p1.y()) << p2 << to;
        } else {
            path << from << p1 << p2 << to;
        }
    } else {
        // LED → Button, LED → LED
        ExitDirection toDir = getExitDirection(toItem, toTerminalIndex);
        qreal approachRun = toLed ? LED_APPROACH : RUN_LENGTH;
        QPointF p3 = toLed ? QPointF(to.x(), to.y() - LED_APPROACH) : approachPoint(to, toDir, RUN_LENGTH);
        qreal p2y = p1.y();
        if (qAbs(p1.y() - p3.y()) < MIN_SEGMENT)
            p2y = (p1.y() < to.y()) ? p3.y() - approachRun : p3.y() + approachRun;
        QPointF p2(p3.x(), p2y);
        if (qAbs(p1.x() - p2.x()) < MIN_SEGMENT) {
            qreal outX = p1.x() + RUN_LENGTH;
            path << from << p1 << QPointF(outX, p1.y()) << p2 << p3 << to;
        } else if (qAbs(p2.y() - p3.y()) >= MIN_SEGMENT) {
            path << from << p1 << p2 << p3 << to;
        } else {
            QPointF p2a(p2.x(), p3.y());
            path << from << p1 << p2 << p2a << p3 << to;
        }
    }
}

QRectF pathBoundingRect(QPointF from, QPointF to,
                        QGraphicsItem *fromItem, int fromTerminalIndex,
                        QGraphicsItem *toItem, int toTerminalIndex) {
    QPolygonF path;
    buildPath(from, to, fromItem, fromTerminalIndex, toItem, toTerminalIndex, path);
    QRectF r = path.boundingRect();
    const qreal margin = 4;
    r = r.adjusted(-margin, -margin, margin, margin);
    const qreal minSize = 6;
    if (r.width() < minSize) r.setWidth(minSize);
    if (r.height() < minSize) r.setHeight(minSize);
    return r;
}

} // namespace

ComponentWire::ComponentWire(QGraphicsItem *fromItem, int fromTerminalIndex,
                             QGraphicsItem *toItem, int toTerminalIndex,
                             QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , fromItem_(fromItem)
    , fromTerminalIndex_(fromTerminalIndex)
    , toItem_(toItem)
    , toTerminalIndex_(toTerminalIndex) {
    setZValue(1);
    setFlag(ItemStacksBehindParent, false);
    setAcceptHoverEvents(true);
    updateEndpoints();
}

ComponentWire::~ComponentWire() = default;

void ComponentWire::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event);
    QToolTip::hideText();
    QGraphicsItem::hoverLeaveEvent(event);
}

QPointF ComponentWire::getTerminalScenePosition(QGraphicsItem *item, int terminalIndex) {
    if (!item)
        return QPointF(0, 0);
    if (LedItem *led = dynamic_cast<LedItem *>(item)) {
        if (terminalIndex == 0)
            return led->mapToScene(led->anodePos());
        if (terminalIndex == 1)
            return led->mapToScene(led->cathodePos());
        return led->mapToScene(led->boundingRect().center());
    }
    if (ResistorItem *res = dynamic_cast<ResistorItem *>(item)) {
        if (terminalIndex == 0)
            return res->mapToScene(res->leg1Pos());
        if (terminalIndex == 1)
            return res->mapToScene(res->leg2Pos());
        return res->mapToScene(res->boundingRect().center());
    }
    if (ButtonItem *btn = dynamic_cast<ButtonItem *>(item)) {
        switch (terminalIndex) {
        case 0: return btn->mapToScene(btn->pin1Pos());
        case 1: return btn->mapToScene(btn->pin2Pos());
        case 2: return btn->mapToScene(btn->pin3Pos());
        case 3: return btn->mapToScene(btn->pin4Pos());
        default: return btn->mapToScene(btn->boundingRect().center());
        }
    }
    return item->mapToScene(item->boundingRect().center());
}

void ComponentWire::updateEndpoints() {
    if (!fromItem_ || !toItem_)
        return;
    prepareGeometryChange();
    fromScene_ = getTerminalScenePosition(fromItem_, fromTerminalIndex_);
    toScene_ = getTerminalScenePosition(toItem_, toTerminalIndex_);
}

QRectF ComponentWire::boundingRect() const {
    return pathBoundingRect(fromScene_, toScene_, fromItem_, fromTerminalIndex_, toItem_, toTerminalIndex_);
}

QColor ComponentWire::wireColor() const {
    if (!fromItem_)
        return QColor(120, 120, 120);
    if (dynamic_cast<LedItem *>(fromItem_))
        return fromTerminalIndex_ == 0 ? QColor(0, 160, 0) : QColor(60, 60, 60);  // anode=green, cathode=gray
    if (dynamic_cast<ResistorItem *>(fromItem_))
        return QColor(200, 120, 0);   // orange
    if (dynamic_cast<ButtonItem *>(fromItem_))
        return QColor(100, 149, 237); // cornflower blue
    return QColor(120, 120, 120);
}

void ComponentWire::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (!fromItem_ || !toItem_)
        return;
    updateEndpoints();
    QPolygonF path;
    buildPath(fromScene_, toScene_, fromItem_, fromTerminalIndex_, toItem_, toTerminalIndex_, path);
    painter->setPen(QPen(wireColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(path);
}
