#include "SimulatorPane.h"
#include "../scene/ArduinoBoardItem.h"
#include "../scene/LedItem.h"
#include "../scene/ResistorItem.h"
#include "../scene/ButtonItem.h"
#include "../scene/ConnectionItem.h"
#include "../scene/ComponentWire.h"
#include "../core/Logger.h"

#include "../scene/PinPositions.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QMenu>
#include <QGraphicsItem>
#include <QEvent>
#include <QGraphicsScene>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QCoreApplication>
#include <QFileInfo>
#include <QToolTip>
#include <QMouseEvent>

SimulatorPane::SimulatorPane(QWidget *parent)
    : QWidget(parent),
    scene(std::make_unique<QGraphicsScene>()),
    view(std::make_unique<QGraphicsView>()) {
    setupUI();
    applyTheme();  // Apply neon blue theme
}

SimulatorPane::~SimulatorPane() = default;

void SimulatorPane::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Title bar
    QWidget *titleBar = new QWidget(this);
    QHBoxLayout *titleBarLayout = new QHBoxLayout(titleBar);
    titleBarLayout->setContentsMargins(4, 4, 4, 4);

    QLabel *titleLabel = new QLabel("Simulator");
    titleLabel->setObjectName("paneTitle");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet(
        "QLabel#paneTitle {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "}"
        );

    titleBarLayout->addWidget(titleLabel, 1);
    titleBar->setLayout(titleBarLayout);

    // Toolbar (buttons placed inside a styled widget)
    setupToolbar();

    QWidget *toolbarWidget = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(4, 4, 4, 4);
    toolbarLayout->setSpacing(8);
    toolbarLayout->addWidget(addComponentButton);
    toolbarLayout->addWidget(removeSelectedButton);
    toolbarLayout->addWidget(zoomMenuButton);
    toolbarLayout->addStretch();

    view->setScene(scene.get());
    view->setRenderHint(QPainter::SmoothPixmapTransform);
    view->setRenderHint(QPainter::Antialiasing);
    view->setRenderHint(QPainter::TextAntialiasing);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setAlignment(Qt::AlignCenter);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view.get(), &QGraphicsView::customContextMenuRequested, this, &SimulatorPane::onContextMenuRequested);
    view->installEventFilter(this);  // update wires on every mouse move during drag

    const qreal margin = 24;
    boardItem = new ArduinoBoardItem();
    scene->addItem(boardItem);
    boardItem->setZValue(-1);
    boardItem->setPos(margin, margin);
    boardItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    boardItem->setFlag(QGraphicsItem::ItemIsMovable, true);

    QRectF boardRect = boardItem->boundingRect();
    scene->setSceneRect(0, 0,
                        boardRect.width() + margin * 2,
                        boardRect.height() + margin * 2);

    layout->addWidget(titleBar);
    layout->addWidget(toolbarWidget);
    layout->addWidget(view.get());

    connectionUpdateTimer = new QTimer(this);
    connect(connectionUpdateTimer, &QTimer::timeout, this, &SimulatorPane::updateConnectionLines);
    connectionUpdateTimer->start(100);

    setLayout(layout);
}

bool SimulatorPane::eventFilter(QObject *watched, QEvent *event) {
    if (watched != view.get())
        return QWidget::eventFilter(watched, event);
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPointF scenePos = view->mapToScene(me->pos());
        QGraphicsItem *under = scene->itemAt(scenePos, view->transform());
        if (!under || (!dynamic_cast<ConnectionItem *>(under) && !dynamic_cast<ComponentWire *>(under)))
            QToolTip::hideText();
    }
    if (event->type() != QEvent::MouseMove)
        return QWidget::eventFilter(watched, event);
    QGraphicsItem *grabbed = scene->mouseGrabberItem();
    if (!grabbed)
        return QWidget::eventFilter(watched, event);
    updateConnectionLines();
    if (grabbed == boardItem) {
        QPointF pos = boardItem->pos();
        QRectF br = boardItem->boundingRect();
        expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
    }
    // Keep movable items inside the scene so they don't go beyond the simulator area
    if (dynamic_cast<ArduinoBoardItem *>(grabbed) || dynamic_cast<LedItem *>(grabbed)
        || dynamic_cast<ResistorItem *>(grabbed) || dynamic_cast<ButtonItem *>(grabbed)) {
        QRectF sr = scene->sceneRect();
        QPointF pos = grabbed->pos();
        QRectF br = grabbed->boundingRect();
        qreal x = qBound(sr.left(), pos.x(), sr.right() - br.width());
        qreal y = qBound(sr.top(), pos.y(), sr.bottom() - br.height());
        if (x != pos.x() || y != pos.y())
            grabbed->setPos(x, y);
    }
    return QWidget::eventFilter(watched, event);
}

void SimulatorPane::setupToolbar() {
    const int iconSize = 24;
    QPixmap plusPix(iconSize, iconSize);
    plusPix.fill(Qt::transparent);
    QPainter p(&plusPix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(QPen(Qt::white, 2.5, Qt::SolidLine, Qt::RoundCap));
    int c = iconSize / 2;
    int hw = 6;
    p.drawLine(c - hw, c, c + hw, c);
    p.drawLine(c, c - hw, c, c + hw);
    p.end();
    QIcon plusIcon(plusPix);

    addComponentButton = new QPushButton(this);
    addComponentButton->setIcon(plusIcon);
    addComponentButton->setIconSize(QSize(iconSize, iconSize));
    addComponentButton->setToolTip("Add a new component");
    addComponentButton->setFixedSize(32, 32);
    addComponentButton->setFocusPolicy(Qt::NoFocus);
    connect(addComponentButton, &QPushButton::clicked, this, &SimulatorPane::showAddComponentMenu);

    removeSelectedButton = new QPushButton(this);
    removeSelectedButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    removeSelectedButton->setIconSize(QSize(18, 18));
    removeSelectedButton->setToolTip("Remove selected");
    removeSelectedButton->setFixedSize(32, 32);

    zoomMenuButton = new QPushButton(QString::fromUtf8("⋮"), this);
    zoomMenuButton->setToolTip("Zoom in, Zoom out");
    zoomMenuButton->setFixedSize(32, 32);

    connect(removeSelectedButton, &QPushButton::clicked, this, &SimulatorPane::onRemoveSelected);
    connect(zoomMenuButton, &QPushButton::clicked, this, &SimulatorPane::showZoomMenu);
}

static QIcon iconForMenu(const QString &resourcePath, const QString &fallbackPath, int size = 28) {
    QPixmap pm(resourcePath);
    if (pm.isNull() && QFileInfo(fallbackPath).exists())
        pm.load(fallbackPath);
    if (!pm.isNull() && (pm.width() > size || pm.height() > size))
        pm = pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return pm.isNull() ? QIcon() : QIcon(pm);
}

void SimulatorPane::showAddComponentMenu() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString resBase = appDir + "/../resources/images/";
    if (!QFileInfo(resBase).isDir())
        resBase = appDir + "/../../resources/images/";
    if (!QFileInfo(resBase).isDir())
        resBase = "resources/images/";

    QMenu menu(this);
    // Menu styling will be handled by applyTheme() via stylesheet inheritance
    menu.addSection("Basic");
    QIcon ledIcon = iconForMenu(":/images/led_on.png", resBase + "led_on.png");
    QIcon pushIcon = iconForMenu(":/images/button.png", resBase + "button.png");
    QIcon resIcon = iconForMenu(":/images/resistor.png", resBase + "resistor.png");
    QAction *ledAct = menu.addAction(ledIcon, "LED");
    QAction *pushAct = menu.addAction(pushIcon, "Pushbutton");
    QAction *resAct = menu.addAction(resIcon, "Resistor");
    connect(ledAct, &QAction::triggered, this, &SimulatorPane::onAddLed);
    connect(pushAct, &QAction::triggered, this, &SimulatorPane::onAddButton);
    connect(resAct, &QAction::triggered, this, &SimulatorPane::onAddResistor);
    menu.exec(addComponentButton->mapToGlobal(addComponentButton->rect().bottomLeft()));
}

void SimulatorPane::onAddLed() {
    int total = totalComponentCount();
    int batch = total / 3;
    int indexInBatch = total % 3;
    LedItem *led = new LedItem(-1);
    scene->addItem(led);
    led->setZValue(2);
    QRectF br = led->boundingRect();
    QPointF pos = componentDropPosition(br.width(), batch, indexInBatch);
    led->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onAddButton() {
    int total = totalComponentCount();
    int batch = total / 3;
    int indexInBatch = total % 3;
    ButtonItem *btn = new ButtonItem();
    scene->addItem(btn);
    buttonOrder_.append(btn);

    btn->setZValue(2);
    QRectF br = btn->boundingRect();
    QPointF pos = componentDropPosition(br.width(), batch, indexInBatch);
    btn->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onAddResistor() {
    int total = totalComponentCount();
    int batch = total / 3;
    int indexInBatch = total % 3;
    ResistorItem *res = new ResistorItem();
    scene->addItem(res);
    resistorOrder_.append(res);
    res->setZValue(2);
    QRectF br = res->boundingRect();
    QPointF pos = componentDropPosition(br.width(), batch, indexInBatch);
    res->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onRemoveSelected() {
    QList<QGraphicsItem *> selected = scene->selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
        QGraphicsItem *item = selected.at(i);
        if (dynamic_cast<ArduinoBoardItem *>(item))
            continue;
        LedItem *led = dynamic_cast<LedItem *>(item);
        ResistorItem *res = dynamic_cast<ResistorItem *>(item);
        ButtonItem *btn = dynamic_cast<ButtonItem *>(item);

        if (led) {
            removeConnectionForLed(led);
            ledOrder_.removeOne(led);
        }
        if (res) {
            removeConnectionForResistor(res);
            resistorOrder_.removeOne(res);
        }
        if (btn) {
            removeConnectionForButton(btn);
            buttonOrder_.removeOne(btn);
        }

        scene->removeItem(item);
        delete item;
        refreshAllComponentWireTooltips();  // renumber tooltips after removal
    }
}

void SimulatorPane::onContextMenuRequested(const QPoint &viewPos) {
    QPointF scenePos = view->mapToScene(viewPos);
    QGraphicsItem *item = scene->itemAt(scenePos, view->transform());
    LedItem *led = item ? dynamic_cast<LedItem *>(item) : nullptr;
    ResistorItem *res = item ? dynamic_cast<ResistorItem *>(item) : nullptr;
    ButtonItem *btn = item ? dynamic_cast<ButtonItem *>(item) : nullptr;

    if (led) {
        QMenu menu(this);
        QMenu *cathodeMenu = menu.addMenu("Set pin (cathode)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = cathodeMenu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, led, pinId]() {
                led->setCathodePinId(pinId);
                setCathodeConnection(led, pinId);
            });
        }
        addConnectToComponentSubmenu(cathodeMenu, led, 1);  // 1 = cathode

        QMenu *anodeMenu = menu.addMenu("Set pin (anode)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = anodeMenu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, led, pinId]() {
                led->setPinId(pinId);
                setAnodeConnection(led, pinId);
            });
        }
        addConnectToComponentSubmenu(anodeMenu, led, 0);  // 0 = anode

        QAction *disconnectAct = menu.addAction("Disconnect all");
        connect(disconnectAct, &QAction::triggered, this, [this, led]() {
            led->setPinId(QString());
            led->setCathodePinId(QString());
            removeConnectionForLed(led);
        });
        menu.exec(view->mapToGlobal(viewPos));
        return;
    }

    if (res) {
        QMenu menu(this);
        QMenu *leg1Menu = menu.addMenu("Set pin (lead1)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = leg1Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, res, pinId]() {
                res->setLeg1PinId(pinId);
                setResistorLeg1Connection(res, pinId);
            });
        }
        addConnectToComponentSubmenu(leg1Menu, res, 0);  // 0 = leg1

        QMenu *leg2Menu = menu.addMenu("Set pin (lead2)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = leg2Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, res, pinId]() {
                res->setLeg2PinId(pinId);
                setResistorLeg2Connection(res, pinId);
            });
        }
        addConnectToComponentSubmenu(leg2Menu, res, 1);  // 1 = leg2

        QAction *disconnectAct = menu.addAction("Disconnect all");
        connect(disconnectAct, &QAction::triggered, this, [this, res]() {
            res->setLeg1PinId(QString());
            res->setLeg2PinId(QString());
            removeConnectionForResistor(res);
        });
        menu.exec(view->mapToGlobal(viewPos));
        return;
    }

    if (btn) {
        QMenu menu(this);
        QMenu *pin1Menu = menu.addMenu("Set pin (pin1)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin1Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin1PinId(pinId);
                setButtonPin1Connection(btn, pinId);
            });
        }
        addConnectToComponentSubmenu(pin1Menu, btn, 0);  // 0 = pin1

        QMenu *pin2Menu = menu.addMenu("Set pin (pin2)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin2Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin2PinId(pinId);
                setButtonPin2Connection(btn, pinId);
            });
        }
        addConnectToComponentSubmenu(pin2Menu, btn, 1);  // 1 = pin2

        QMenu *pin3Menu = menu.addMenu("Set pin (pin3)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin3Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin3PinId(pinId);
                setButtonPin3Connection(btn, pinId);
            });
        }
        addConnectToComponentSubmenu(pin3Menu, btn, 2);  // 2 = pin3

        QMenu *pin4Menu = menu.addMenu("Set pin (pin4)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin4Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin4PinId(pinId);
                setButtonPin4Connection(btn, pinId);
            });
        }
        addConnectToComponentSubmenu(pin4Menu, btn, 3);  // 3 = pin4

        QAction *disconnectAct = menu.addAction("Disconnect all");
        connect(disconnectAct, &QAction::triggered, this, [this, btn]() {
            btn->setPin1PinId(QString());
            btn->setPin2PinId(QString());
            btn->setPin3PinId(QString());
            btn->setPin4PinId(QString());
            removeConnectionForButton(btn);
        });
        menu.exec(view->mapToGlobal(viewPos));
    }
}

void SimulatorPane::setAnodeConnection(LedItem *led, const QString &pinId) {
    auto it = ledAnodeConnections.find(led);
    if (it != ledAnodeConnections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        ledAnodeConnections.erase(it);
    }
    removeComponentWiresFromTerminal(led, 0);  // 0 = anode
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, led, true);  // true = anode
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(led, 0)));
    ledAnodeConnections[led] = conn;
}

void SimulatorPane::setCathodeConnection(LedItem *led, const QString &pinId) {
    auto it = ledCathodeConnections.find(led);
    if (it != ledCathodeConnections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        ledCathodeConnections.erase(it);
    }
    removeComponentWiresFromTerminal(led, 1);  // 1 = cathode
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, led, false);  // false = cathode
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(led, 1)));
    ledCathodeConnections[led] = conn;
}

void SimulatorPane::removeConnectionForLed(LedItem *led) {
    auto itA = ledAnodeConnections.find(led);
    if (itA != ledAnodeConnections.end()) {
        scene->removeItem(itA.value());
        delete itA.value();
        ledAnodeConnections.erase(itA);
    }
    auto itC = ledCathodeConnections.find(led);
    if (itC != ledCathodeConnections.end()) {
        scene->removeItem(itC.value());
        delete itC.value();
        ledCathodeConnections.erase(itC);
    }
    removeComponentWiresForComponent(led);

}

void SimulatorPane::setResistorLeg1Connection(ResistorItem *res, const QString &pinId) {
    auto it = resistorLeg1Connections.find(res);
    if (it != resistorLeg1Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        resistorLeg1Connections.erase(it);
    }
    removeComponentWiresFromTerminal(res, 0);  // 0 = leg1

    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, res, 1);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(res, 0)));

    resistorLeg1Connections[res] = conn;
}

void SimulatorPane::setResistorLeg2Connection(ResistorItem *res, const QString &pinId) {
    auto it = resistorLeg2Connections.find(res);
    if (it != resistorLeg2Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        resistorLeg2Connections.erase(it);
    }
    removeComponentWiresFromTerminal(res, 1);  // 1 = leg2

    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, res, 2);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(res, 1)));

    resistorLeg2Connections[res] = conn;
}

void SimulatorPane::removeConnectionForResistor(ResistorItem *res) {
    auto it1 = resistorLeg1Connections.find(res);
    if (it1 != resistorLeg1Connections.end()) {
        scene->removeItem(it1.value());
        delete it1.value();
        resistorLeg1Connections.erase(it1);
    }
    auto it2 = resistorLeg2Connections.find(res);
    if (it2 != resistorLeg2Connections.end()) {
        scene->removeItem(it2.value());
        delete it2.value();
        resistorLeg2Connections.erase(it2);
    }
    removeComponentWiresForComponent(res);

}

void SimulatorPane::setButtonPin1Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin1Connections.find(btn);
    if (it != buttonPin1Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin1Connections.erase(it);
    }
    removeComponentWiresFromTerminal(btn, 0);  // 0 = pin1

    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 1);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(btn, 0)));

    buttonPin1Connections[btn] = conn;
}

void SimulatorPane::setButtonPin2Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin2Connections.find(btn);
    if (it != buttonPin2Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin2Connections.erase(it);
    }
    removeComponentWiresFromTerminal(btn, 1);  // 1 = pin2

    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 2);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(btn, 1)));

    buttonPin2Connections[btn] = conn;
}

void SimulatorPane::setButtonPin3Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin3Connections.find(btn);
    if (it != buttonPin3Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin3Connections.erase(it);
    }
    removeComponentWiresFromTerminal(btn, 2);  // 2 = pin3

    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 3);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(btn, 2)));

    buttonPin3Connections[btn] = conn;
}

void SimulatorPane::setButtonPin4Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin4Connections.find(btn);
    if (it != buttonPin4Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin4Connections.erase(it);
    }
    removeComponentWiresFromTerminal(btn, 3);  // 3 = pin4

    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 4);
    scene->addItem(conn);
    conn->setZValue(1);
    conn->setToolTip(QString("Pin %1 → %2").arg(pinId, getComponentTerminalLabel(btn, 3)));

    buttonPin4Connections[btn] = conn;
}

void SimulatorPane::removeConnectionForButton(ButtonItem *btn) {
    for (auto *map : { &buttonPin1Connections, &buttonPin2Connections, &buttonPin3Connections, &buttonPin4Connections }) {
        auto it = map->find(btn);
        if (it != map->end()) {
            scene->removeItem(it.value());
            delete it.value();
            map->erase(it);
        }
    }
    removeComponentWiresForComponent(btn);

}

void SimulatorPane::removeComponentWiresFromTerminal(QGraphicsItem *item, int terminalIndex) {
    for (auto it = componentWires.begin(); it != componentWires.end(); ) {
        ComponentWire *w = *it;
        bool fromMatch = (w->fromItem() == item && w->fromTerminalIndex() == terminalIndex);
        bool toMatch = (w->toItem() == item && w->toTerminalIndex() == terminalIndex);
        if (fromMatch || toMatch) {
            scene->removeItem(w);
            delete w;
            it = componentWires.erase(it);
        } else {
            ++it;
        }
    }
}

void SimulatorPane::removeComponentWiresForComponent(QGraphicsItem *item) {
    for (auto it = componentWires.begin(); it != componentWires.end(); ) {
        ComponentWire *w = *it;
        if (w->fromItem() == item || w->toItem() == item) {
            scene->removeItem(w);
            delete w;
            it = componentWires.erase(it);
        } else {
            ++it;
        }
    }
}

QString SimulatorPane::getComponentTerminalLabel(QGraphicsItem *item, int terminalIndex) const {
    if (!item)
        return QString("?");
    if (LedItem *led = dynamic_cast<LedItem *>(item)) {
        QString term = (terminalIndex == 0) ? "A" : "C";
        if (ledOrder_.size() > 1) {
            int idx = ledOrder_.indexOf(led);
            int n = (idx >= 0) ? (idx + 1) : 0;
            return QString("LED%1:%2").arg(QString::number(n), term);
        }
        return QString("LED:%1").arg(term);
    }
    if (ResistorItem *res = dynamic_cast<ResistorItem *>(item)) {
        QString term = (terminalIndex == 0) ? "L1" : "L2";
        if (resistorOrder_.size() > 1) {
            int idx = resistorOrder_.indexOf(res);
            int n = (idx >= 0) ? (idx + 1) : 0;
            return QString("R%1:%2").arg(QString::number(n), term);
        }
        return QString("R:%1").arg(term);
    }
    if (ButtonItem *btn = dynamic_cast<ButtonItem *>(item)) {
        QString term = QString("P%1").arg(terminalIndex + 1);
        if (buttonOrder_.size() > 1) {
            int idx = buttonOrder_.indexOf(btn);
            int n = (idx >= 0) ? (idx + 1) : 0;
            return QString("Btn%1:%2").arg(QString::number(n), term);
        }
        return QString("Btn:%1").arg(term);
    }
    return QString("Component:T%1").arg(terminalIndex + 1);
}

void SimulatorPane::refreshAllComponentWireTooltips() {
    for (ComponentWire *w : componentWires) {
        if (!w->fromItem() || !w->toItem())
            continue;
        QString tip = getComponentTerminalLabel(w->fromItem(), w->fromTerminalIndex()) + " → "
                      + getComponentTerminalLabel(w->toItem(), w->toTerminalIndex());
        w->setToolTip(tip);
    }
}

void SimulatorPane::addComponentWire(QGraphicsItem *fromItem, int fromTerminalIndex,
                                     QGraphicsItem *toItem, int toTerminalIndex) {
    ComponentWire *w = new ComponentWire(fromItem, fromTerminalIndex, toItem, toTerminalIndex);
    scene->addItem(w);
    w->setZValue(1);
    QString tip = getComponentTerminalLabel(fromItem, fromTerminalIndex) + " → "
                  + getComponentTerminalLabel(toItem, toTerminalIndex);
    w->setToolTip(tip);
    componentWires.append(w);
}

void SimulatorPane::addConnectToComponentSubmenu(QMenu *menu, QGraphicsItem *fromItem, int fromTerminalIndex) {
    // Use insertion-order lists so numbering is first-added = 1, second = 2, etc.
    if (ledOrder_.isEmpty() && resistorOrder_.isEmpty() && buttonOrder_.isEmpty())
        return;
    menu->addSeparator();
    for (int i = 0; i < ledOrder_.size(); ++i) {
        LedItem *toLed = ledOrder_[i];
        if (toLed == fromItem)
            continue;
        int n = i + 1;  // 1-based: first-added = LED 1
        QString label = ledOrder_.size() > 1 ? QString("LED%1:C").arg(n) : "LED:C";
        QAction *act = menu->addAction(label);
        connect(act, &QAction::triggered, this, [this, fromItem, fromTerminalIndex, toLed]() {
            removeComponentWiresFromTerminal(fromItem, fromTerminalIndex);
            addComponentWire(fromItem, fromTerminalIndex, toLed, 1);
        });
        label = ledOrder_.size() > 1 ? QString("LED%1:A").arg(n) : "LED:A";
        act = menu->addAction(label);
        connect(act, &QAction::triggered, this, [this, fromItem, fromTerminalIndex, toLed]() {
            removeComponentWiresFromTerminal(fromItem, fromTerminalIndex);
            addComponentWire(fromItem, fromTerminalIndex, toLed, 0);
        });
    }
    for (int i = 0; i < resistorOrder_.size(); ++i) {
        ResistorItem *toRes = resistorOrder_[i];
        if (toRes == fromItem)
            continue;
        int n = i + 1;
        QString label = resistorOrder_.size() > 1 ? QString("R%1:L1").arg(n) : "R:L1";
        QAction *act = menu->addAction(label);
        connect(act, &QAction::triggered, this, [this, fromItem, fromTerminalIndex, toRes]() {
            removeComponentWiresFromTerminal(fromItem, fromTerminalIndex);
            addComponentWire(fromItem, fromTerminalIndex, toRes, 0);
        });
        label = resistorOrder_.size() > 1 ? QString("R%1:L2").arg(n) : "R:L2";
        act = menu->addAction(label);
        connect(act, &QAction::triggered, this, [this, fromItem, fromTerminalIndex, toRes]() {
            removeComponentWiresFromTerminal(fromItem, fromTerminalIndex);
            addComponentWire(fromItem, fromTerminalIndex, toRes, 1);
        });
    }
    for (int i = 0; i < buttonOrder_.size(); ++i) {
        ButtonItem *toBtn = buttonOrder_[i];
        if (toBtn == fromItem)
            continue;
        int n = i + 1;
        for (int p = 1; p <= 4; ++p) {
            QString label = buttonOrder_.size() > 1
                                ? QString("Btn%1:P%2").arg(QString::number(n), QString::number(p))
                                : QString("Btn:P%1").arg(p);
            QAction *act = menu->addAction(label);
            int toTerm = p - 1;
            connect(act, &QAction::triggered, this, [this, fromItem, fromTerminalIndex, toBtn, toTerm]() {
                removeComponentWiresFromTerminal(fromItem, fromTerminalIndex);
                addComponentWire(fromItem, fromTerminalIndex, toBtn, toTerm);
            });
        }
    }
}

void SimulatorPane::updateConnectionLines() {
    for (ConnectionItem *conn : ledAnodeConnections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : ledCathodeConnections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : resistorLeg1Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : resistorLeg2Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : buttonPin1Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : buttonPin2Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : buttonPin3Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ConnectionItem *conn : buttonPin4Connections.values()) {
        conn->updateEndpoints();
        conn->update();
    }
    for (ComponentWire *w : componentWires) {
        w->updateEndpoints();
        w->update();
    }
}

void SimulatorPane::showZoomMenu() {
    QMenu menu(this);
    QAction *zoomInAct = menu.addAction("Zoom In");
    QAction *zoomOutAct = menu.addAction("Zoom Out");
    connect(zoomInAct, &QAction::triggered, this, &SimulatorPane::onZoomIn);
    connect(zoomOutAct, &QAction::triggered, this, &SimulatorPane::onZoomOut);
    menu.exec(zoomMenuButton->mapToGlobal(zoomMenuButton->rect().bottomLeft()));
}

void SimulatorPane::onZoomIn() {
    const qreal step = 1.25;
    zoomFactor_ *= step;
    zoomFactor_ = qMin(zoomFactor_, 4.0);
    view->scale(step, step);
}

void SimulatorPane::onZoomOut() {
    const qreal step = 0.8;
    zoomFactor_ *= step;
    zoomFactor_ = qMax(zoomFactor_, 0.25);
    view->scale(step, step);
}

void SimulatorPane::expandSceneRectIfNeeded(qreal x, qreal y, qreal w, qreal h) {
    QRectF r = scene->sceneRect();
    qreal right = x + w + 20;
    qreal bottom = y + h + 20;
    if (right > r.right() || bottom > r.bottom()) {
        r.setRight(qMax(r.right(), right));
        r.setBottom(qMax(r.bottom(), bottom));
        scene->setSceneRect(r);
        fitViewToScene();
    }
}

void SimulatorPane::fitViewToScene() {
    view->resetTransform();
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    if (zoomFactor_ != 1.0)
        view->scale(zoomFactor_, zoomFactor_);
}
int SimulatorPane::totalComponentCount() const {
    return ledOrder_.size() + resistorOrder_.size() + buttonOrder_.size();
}
QPointF SimulatorPane::componentDropPosition(qreal itemWidth, int batch, int indexInBatch) const {
    Q_UNUSED(indexInBatch);
    QRectF visibleScene = view->mapToScene(view->viewport()->rect()).boundingRect();
    qreal centerX = visibleScene.center().x() - itemWidth / 2;
    qreal topY = visibleScene.top() + 20;
    // Each batch of three: all on top of each other at one position. Batch 0 = center, batch 1 = next column right, etc.
    const qreal columnOffset = 80;
    qreal x = centerX + batch * columnOffset;
    qreal y = topY;
    return QPointF(x, y);
}

void SimulatorPane::updateToolbarButtonPositions() {
    // No longer needed because toolbar is managed by layout.
}

void SimulatorPane::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    fitViewToScene();
    // No manual button positioning – layout handles it.
}

int SimulatorPane::getButtonStateForPin(int pinNumber) const {
    // Button through component wires (e.g. D2 → resistor → button) same as LED
    if (pinNumber >= 0 && pinNumber <= 13) {
        QString pinId = QString("D%1").arg(pinNumber);
        QList<TerminalRef> direct = getTerminalsConnectedToPin(pinId);
        QList<TerminalRef> reachable = getReachableTerminals(direct);
        for (const TerminalRef &t : reachable) {
            ButtonItem *btn = dynamic_cast<ButtonItem *>(t.first);
            if (btn)
                return btn->isPressed() ? 0 : 1;  // first button found on this net
        }
    }
    return 1;  // HIGH when no button (pull-up default)
}

void SimulatorPane::setSimulationRunning(bool running) {
    // Buttons: switch between design mode (drag/select/context-menu) and simulation mode (press/release)
    for (ButtonItem *btn : buttonOrder_)
        btn->setSimulationMode(running);

    // Lock all other components in place during simulation so they can't be dragged
    for (LedItem *led : ledOrder_) {
        led->setFlag(QGraphicsItem::ItemIsMovable, !running);
        led->setFlag(QGraphicsItem::ItemIsSelectable, !running);
    }
    for (ResistorItem *res : resistorOrder_) {
        res->setFlag(QGraphicsItem::ItemIsMovable, !running);
        res->setFlag(QGraphicsItem::ItemIsSelectable, !running);
    }
    if (boardItem) {
        boardItem->setFlag(QGraphicsItem::ItemIsMovable, !running);
        boardItem->setFlag(QGraphicsItem::ItemIsSelectable, !running);
    }
}

void SimulatorPane::resetAllLeds() {
    for (LedItem *led : ledOrder_)
        led->setState(false);
    if (scene)
        scene->update();
}

// Connectivity resolution: pin → component terminals

QList<SimulatorPane::TerminalRef> SimulatorPane::getTerminalsConnectedToPin(const QString &pinId) const {
    QList<TerminalRef> out;
    auto add = [&out](QGraphicsItem *item, int term) {
        if (item)
            out.append(qMakePair(item, term));
    };
    for (auto it = ledAnodeConnections.cbegin(); it != ledAnodeConnections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), it.value()->isToAnode() ? 0 : 1);
    }
    for (auto it = ledCathodeConnections.cbegin(); it != ledCathodeConnections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), it.value()->isToAnode() ? 0 : 1);
    }
    for (auto it = resistorLeg1Connections.cbegin(); it != resistorLeg1Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 0);
    }
    for (auto it = resistorLeg2Connections.cbegin(); it != resistorLeg2Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 1);
    }
    for (auto it = buttonPin1Connections.cbegin(); it != buttonPin1Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 0);
    }
    for (auto it = buttonPin2Connections.cbegin(); it != buttonPin2Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 1);
    }
    for (auto it = buttonPin3Connections.cbegin(); it != buttonPin3Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 2);
    }
    for (auto it = buttonPin4Connections.cbegin(); it != buttonPin4Connections.cend(); ++it) {
        if (it.value()->pinId() == pinId)
            add(it.key(), 3);
    }
    return out;
}

QList<SimulatorPane::TerminalRef> SimulatorPane::getReachableTerminals(const QList<TerminalRef> &start) const {
    QList<TerminalRef> result = start;
    QSet<QPair<void *, int>> seen;
    for (const TerminalRef &t : start)
        seen.insert(qMakePair(static_cast<void *>(t.first), t.second));
    int idx = 0;
    while (idx < result.size()) {
        QGraphicsItem *item = result.at(idx).first;
        int term = result.at(idx).second;
        ++idx;

        // Resistor: both legs are electrical node — add the other leg so wires from it are followed
        if (ResistorItem *res = dynamic_cast<ResistorItem *>(item)) {
            Q_UNUSED(res);
            int otherLeg = 1 - term;
            auto key = qMakePair(static_cast<void *>(item), otherLeg);
            if (!seen.contains(key)) {
                seen.insert(key);
                result.append(qMakePair(item, otherLeg));
            }
        }

        for (ComponentWire *w : componentWires) {
            if (!w->fromItem() || !w->toItem())
                continue;
            QGraphicsItem *other = nullptr;
            int otherTerm = -1;
            if (w->fromItem() == item && w->fromTerminalIndex() == term) {
                other = w->toItem();
                otherTerm = w->toTerminalIndex();
            } else if (w->toItem() == item && w->toTerminalIndex() == term) {
                other = w->fromItem();
                otherTerm = w->fromTerminalIndex();
            }
            if (other && otherTerm >= 0) {
                auto key = qMakePair(static_cast<void *>(other), otherTerm);
                if (!seen.contains(key)) {
                    seen.insert(key);
                    result.append(qMakePair(other, otherTerm));
                }
            }
        }
    }
    return result;
}

QMap<int, LedItem *> SimulatorPane::getPinToLedAnodeMap() const {
    QMap<int, LedItem *> map;
    for (int pinNum = 0; pinNum <= 13; ++pinNum) {
        QString pinId = QString("D%1").arg(pinNum);
        QList<TerminalRef> direct = getTerminalsConnectedToPin(pinId);
        QList<TerminalRef> reachable = getReachableTerminals(direct);
        for (const TerminalRef &t : reachable) {
            LedItem *led = dynamic_cast<LedItem *>(t.first);
            if (led && t.second == 0) {  // 0 = anode
                map[pinNum] = led;
                break;  // one LED per pin (first found)
            }
        }
    }
    return map;
}

void SimulatorPane::applyTheme() {
    bool light = false; // Simulator always uses dark? Could be toggled later.

    // Apply neon blue theme to the whole pane and its children.
    if (!light) {
        setStyleSheet(R"(
            SimulatorPane {
                background-color: #0a0c10;
            }
            QLabel#paneTitle {
                color: #00b7ff;
                background-color: #14161a;
                border: 1px solid #00b7ff;
                border-radius: 4px;
                padding: 4px;
            }
            QWidget {
                background-color: transparent;
                color: #b0e0ff;
            }
            QPushButton {
                background-color: #14161a;
                color: #b0e0ff;
                border: 1px solid #00b7ff;
                border-radius: 4px;
                padding: 4px;
            }
            QPushButton:hover {
                background-color: #1e2028;
                border-color: #33ccff;
            }
            QPushButton:pressed {
                background-color: #0f1115;
            }
            QMenu {
                background-color: #14161a;
                color: #b0e0ff;
                border: 1px solid #00b7ff;
            }
            QMenu::item {
                padding: 4px 16px;
            }
            QMenu::item:selected {
                background-color: #007acc;
            }
            QGraphicsView {
                border: 1px solid #00b7ff;
                border-radius: 4px;
                background-color: #14161a;
            }
        )");
    } else {
        // Light variant (keep consistent with other panes)
        setStyleSheet(R"(
            SimulatorPane {
                background-color: #e6f0ff;
            }
            QLabel#paneTitle {
                color: #0099ff;
                background-color: #ffffff;
                border: 1px solid #0099ff;
                border-radius: 4px;
                padding: 4px;
            }
            QWidget {
                background-color: transparent;
                color: #00264d;
            }
            QPushButton {
                background-color: #ffffff;
                color: #00264d;
                border: 1px solid #0099ff;
                border-radius: 4px;
                padding: 4px;
            }
            QPushButton:hover {
                background-color: #f0f7ff;
            }
            QPushButton:pressed {
                background-color: #d9e9ff;
            }
            QMenu {
                background-color: #ffffff;
                color: #00264d;
                border: 1px solid #0099ff;
            }
            QMenu::item:selected {
                background-color: #b3d9ff;
            }
            QGraphicsView {
                border: 1px solid #0099ff;
                border-radius: 4px;
                background-color: #ffffff;
            }
        )");
    }
}
