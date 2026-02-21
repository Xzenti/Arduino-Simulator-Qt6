#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QPalette>
#include <QColor>
#include <QString>

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum Theme {
        Light,
        Dark
    };
    Q_ENUM(Theme)

    // Singleton accessor
    static ThemeManager& instance() {
        static ThemeManager _instance;
        return _instance;
    }

    Theme currentTheme() const { return m_currentTheme; }
    void setTheme(Theme theme);
    void toggleTheme();

    // Color getters for consistent theming
    QPalette getApplicationPalette() const;
    QString getEditorStyleSheet() const;
    QString getOutputPaneStyleSheet() const;
    QString getButtonStyleSheet() const;
    QString getLineNumberStyleSheet() const;

    // Specific colors
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor borderColor() const;
    QColor highlightColor() const;
    QColor lineHighlightColor() const;
    QColor selectionColor() const;

    // Log colors
    QColor logDebugColor() const;
    QColor logInfoColor() const;
    QColor logWarningColor() const;
    QColor logErrorColor() const;

signals:
    void themeChanged(Theme newTheme);

private:
    ThemeManager();
    ~ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    Theme m_currentTheme;
};

#endif // THEMEMANAGER_H
