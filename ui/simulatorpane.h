#ifndef SIMULATORPANE_H
#define SIMULATORPANE_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QResizeEvent>
#include <QMap>
#include <QTimer>
#include <memory>
#include <QList>
#include <QPair>
#include <QSet>

class QPushButton;
class ArduinoBoardItem;
class QMenu;
class BuzzerItem;
class ComponentWire;

class LedItem;
class ResistorItem;
class ButtonItem;
class ConnectionItem;

class SimulatorPane : public QWidget {
    Q_OBJECT

public:
    SimulatorPane(QWidget *parent = nullptr);
    ~SimulatorPane();

    QGraphicsScene *getScene() const { return scene.get(); }
    QGraphicsView *getView() const { return view.get(); }
    void applyTheme();  // Stub for now

    QMap<int, LedItem *> getPinToLedAnodeMap() const;
    int getButtonStateForPin(int pinNumber) const;

    void setSimulationRunning(bool running);
    // Turn off all LEDs (call when simulation stops)
    void resetAllLeds();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAddLed();
    void onAddButton();
    void onAddResistor();
    void onRemoveSelected();
    void onContextMenuRequested(const QPoint &viewPos);
    void updateConnectionLines();
    void onZoomIn();
    void onZoomOut();
    void showZoomMenu();
    void onAddBuzzer();

    void showAddComponentMenu();

private:
    void setupUI();
    void setupToolbar();
    void expandSceneRectIfNeeded(qreal x, qreal y, qreal w, qreal h);
    void fitViewToScene();
    int totalComponentCount() const;
    QPointF componentDropPosition(qreal itemWidth, int batch, int indexInBatch) const;
    void updateToolbarButtonPositions();
    void setAnodeConnection(LedItem *led, const QString &pinId);
    void setCathodeConnection(LedItem *led, const QString &pinId);
    void removeConnectionForLed(LedItem *led);
    void setResistorLeg1Connection(ResistorItem *res, const QString &pinId);
    void setResistorLeg2Connection(ResistorItem *res, const QString &pinId);
    void removeConnectionForResistor(ResistorItem *res);
    void setButtonPin1Connection(ButtonItem *btn, const QString &pinId);
    void setButtonPin2Connection(ButtonItem *btn, const QString &pinId);
    void setButtonPin3Connection(ButtonItem *btn, const QString &pinId);
    void setButtonPin4Connection(ButtonItem *btn, const QString &pinId);
    void removeConnectionForButton(ButtonItem *btn);

    void removeComponentWiresFromTerminal(QGraphicsItem *item, int terminalIndex);
    void removeComponentWiresForComponent(QGraphicsItem *item);
    void addComponentWire(QGraphicsItem *fromItem, int fromTerminalIndex,
                          QGraphicsItem *toItem, int toTerminalIndex);
    void addConnectToComponentSubmenu(QMenu *menu, QGraphicsItem *fromItem, int fromTerminalIndex);
    QString getComponentTerminalLabel(QGraphicsItem *item, int terminalIndex) const;
    void refreshAllComponentWireTooltips();

    // Resolve which component terminals are electrically connected to a board pin (follows component wires)
    using TerminalRef = QPair<QGraphicsItem *, int>;
    QList<TerminalRef> getTerminalsConnectedToPin(const QString &pinId) const;
    QList<TerminalRef> getReachableTerminals(const QList<TerminalRef> &start) const;


    std::unique_ptr<QGraphicsScene> scene;
    std::unique_ptr<QGraphicsView> view;
    ArduinoBoardItem *boardItem = nullptr;
    QPushButton *addComponentButton = nullptr;
    QPushButton *removeSelectedButton = nullptr;
    QPushButton *zoomMenuButton = nullptr;

    qreal zoomFactor_ = 1.0;

    QMap<LedItem *, ConnectionItem *> ledAnodeConnections;
    QMap<LedItem *, ConnectionItem *> ledCathodeConnections;
    QMap<ResistorItem *, ConnectionItem *> resistorLeg1Connections;
    QMap<ResistorItem *, ConnectionItem *> resistorLeg2Connections;
    QMap<ButtonItem *, ConnectionItem *> buttonPin1Connections;
    QMap<ButtonItem *, ConnectionItem *> buttonPin2Connections;
    QMap<ButtonItem *, ConnectionItem *> buttonPin3Connections;
    QMap<ButtonItem *, ConnectionItem *> buttonPin4Connections;
    QList<ComponentWire *> componentWires;

    QTimer *connectionUpdateTimer = nullptr;

    QList<LedItem *> ledOrder_;
    QList<ResistorItem *> resistorOrder_;
    QList<ButtonItem *> buttonOrder_;
    QList<BuzzerItem *> buzzerOrder_;
};

#endif // SIMULATORPANE_H
