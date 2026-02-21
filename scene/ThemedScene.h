// ThemedScene.h
#pragma once
#include <QGraphicsScene>
#include <QPainter>
#include "ThemeManager.h"

class ThemedScene : public QGraphicsScene {
public:
    explicit ThemedScene(QObject* parent = nullptr) : QGraphicsScene(parent) {}

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override {
        bool light = ThemeManager::instance().backgroundColor() == QColor(255, 255, 255);
        QColor gridColor = light ? QColor(220, 220, 220) : QColor(50, 50, 50);

        painter->fillRect(rect, ThemeManager::instance().backgroundColor());
        painter->setPen(gridColor);

        const int gridSize = 25;
        for (int x = int(rect.left()); x < rect.right(); x += gridSize)
            painter->drawLine(x, rect.top(), x, rect.bottom());

        for (int y = int(rect.top()); y < rect.bottom(); y += gridSize)
            painter->drawLine(rect.left(), y, rect.right(), y);
    }
};
