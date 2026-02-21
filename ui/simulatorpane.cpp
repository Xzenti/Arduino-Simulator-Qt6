#include "SimulatorPane.h"
#include "../scene/ArduinoBoardItem.h"
#include "../scene/LedItem.h"
#include "../scene/ResistorItem.h"
#include "../scene/ButtonItem.h"
#include "../scene/ConnectionItem.h"
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
    if (watched != view.get() || event->type() != QEvent::MouseMove)
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
    int count = 0;
    for (QGraphicsItem *item : scene->items()) {
        if (dynamic_cast<LedItem *>(item))
            count++;
    }
    LedItem *led = new LedItem(-1);
    scene->addItem(led);
    led->setZValue(2);
    QRectF br = led->boundingRect();
    QPointF pos = componentDropPosition(br.width(), count);
    led->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onAddButton() {
    int count = 0;
    for (QGraphicsItem *item : scene->items()) {
        if (dynamic_cast<ButtonItem *>(item))
            count++;
    }
    ButtonItem *btn = new ButtonItem();
    scene->addItem(btn);
    btn->setZValue(2);
    QRectF br = btn->boundingRect();
    QPointF pos = componentDropPosition(br.width(), count);
    btn->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onAddResistor() {
    int count = 0;
    for (QGraphicsItem *item : scene->items()) {
        if (dynamic_cast<ResistorItem *>(item))
            count++;
    }
    ResistorItem *res = new ResistorItem();
    scene->addItem(res);
    res->setZValue(2);
    QRectF br = res->boundingRect();
    QPointF pos = componentDropPosition(br.width(), count);
    res->setPos(pos);
    expandSceneRectIfNeeded(pos.x(), pos.y(), br.width(), br.height());
}

void SimulatorPane::onRemoveSelected() {
    QList<QGraphicsItem *> selected = scene->selectedItems();
    for (QGraphicsItem *item : selected) {
        if (dynamic_cast<ArduinoBoardItem *>(item))
            continue;
        LedItem *led = dynamic_cast<LedItem *>(item);
        ResistorItem *res = dynamic_cast<ResistorItem *>(item);
        ButtonItem *btn = dynamic_cast<ButtonItem *>(item);
        if (led)
            removeConnectionForLed(led);
        if (res)
            removeConnectionForResistor(res);
        if (btn)
            removeConnectionForButton(btn);
        scene->removeItem(item);
        delete item;
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
        QMenu *anodeMenu = menu.addMenu("Set pin (anode)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = anodeMenu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, led, pinId]() {
                led->setPinId(pinId);
                setAnodeConnection(led, pinId);
            });
        }
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
        QMenu *leg2Menu = menu.addMenu("Set pin (lead2)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = leg2Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, res, pinId]() {
                res->setLeg2PinId(pinId);
                setResistorLeg2Connection(res, pinId);
            });
        }
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
        QMenu *pin2Menu = menu.addMenu("Set pin (pin2)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin2Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin2PinId(pinId);
                setButtonPin2Connection(btn, pinId);
            });
        }
        QMenu *pin3Menu = menu.addMenu("Set pin (pin3)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin3Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin3PinId(pinId);
                setButtonPin3Connection(btn, pinId);
            });
        }
        QMenu *pin4Menu = menu.addMenu("Set pin (pin4)");
        for (const QString &pinId : PinPositions::getPinIdList()) {
            QAction *act = pin4Menu->addAction(pinId);
            connect(act, &QAction::triggered, this, [this, btn, pinId]() {
                btn->setPin4PinId(pinId);
                setButtonPin4Connection(btn, pinId);
            });
        }
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
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, led, true);
    scene->addItem(conn);
    conn->setZValue(1);
    ledAnodeConnections[led] = conn;
}

void SimulatorPane::setCathodeConnection(LedItem *led, const QString &pinId) {
    auto it = ledCathodeConnections.find(led);
    if (it != ledCathodeConnections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        ledCathodeConnections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, led, false);
    scene->addItem(conn);
    conn->setZValue(1);
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
}

void SimulatorPane::setResistorLeg1Connection(ResistorItem *res, const QString &pinId) {
    auto it = resistorLeg1Connections.find(res);
    if (it != resistorLeg1Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        resistorLeg1Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, res, 1);
    scene->addItem(conn);
    conn->setZValue(1);
    resistorLeg1Connections[res] = conn;
}

void SimulatorPane::setResistorLeg2Connection(ResistorItem *res, const QString &pinId) {
    auto it = resistorLeg2Connections.find(res);
    if (it != resistorLeg2Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        resistorLeg2Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem)
        return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, res, 2);
    scene->addItem(conn);
    conn->setZValue(1);
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
}

void SimulatorPane::setButtonPin1Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin1Connections.find(btn);
    if (it != buttonPin1Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin1Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 1);
    scene->addItem(conn);
    conn->setZValue(1);
    buttonPin1Connections[btn] = conn;
}

void SimulatorPane::setButtonPin2Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin2Connections.find(btn);
    if (it != buttonPin2Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin2Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 2);
    scene->addItem(conn);
    conn->setZValue(1);
    buttonPin2Connections[btn] = conn;
}

void SimulatorPane::setButtonPin3Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin3Connections.find(btn);
    if (it != buttonPin3Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin3Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 3);
    scene->addItem(conn);
    conn->setZValue(1);
    buttonPin3Connections[btn] = conn;
}

void SimulatorPane::setButtonPin4Connection(ButtonItem *btn, const QString &pinId) {
    auto it = buttonPin4Connections.find(btn);
    if (it != buttonPin4Connections.end()) {
        scene->removeItem(it.value());
        delete it.value();
        buttonPin4Connections.erase(it);
    }
    if (pinId.isEmpty() || !boardItem) return;
    ConnectionItem *conn = new ConnectionItem(boardItem, pinId, btn, 4);
    scene->addItem(conn);
    conn->setZValue(1);
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

QPointF SimulatorPane::componentDropPosition(qreal itemWidth, int stackIndex) const {
    QRectF visibleScene = view->mapToScene(view->viewport()->rect()).boundingRect();
    qreal centerX = visibleScene.center().x() - itemWidth / 2;
    qreal topY = visibleScene.top() + 20 + stackIndex * 55;
    return QPointF(centerX, topY);
}

void SimulatorPane::updateToolbarButtonPositions() {
    // No longer needed because toolbar is managed by layout.
}

void SimulatorPane::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    fitViewToScene();
    // No manual button positioning – layout handles it.
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
