#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// QML singleton bridging the theme preference (AppConfig), the resolved
// dark/light state (following the OS color scheme when the preference is
// "system") and the Palette colors. Palette::setMode is called here so
// category colors resolve to their dark-mode steps everywhere at once.
class ThemeController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString preference READ preference WRITE setPreference
                   NOTIFY preferenceChanged)
    Q_PROPERTY(bool dark READ dark NOTIFY darkChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY darkChanged)
    Q_PROPERTY(QColor primaryInk READ primaryInk NOTIFY darkChanged)
    Q_PROPERTY(QColor secondaryInk READ secondaryInk NOTIFY darkChanged)
    Q_PROPERTY(QColor mutedInk READ mutedInk NOTIFY darkChanged)
    Q_PROPERTY(QColor gridline READ gridline NOTIFY darkChanged)
    Q_PROPERTY(QColor axisLine READ axisLine NOTIFY darkChanged)
    Q_PROPERTY(QColor good READ good NOTIFY darkChanged)
    Q_PROPERTY(QColor serious READ serious NOTIFY darkChanged)
    Q_PROPERTY(QColor critical READ critical NOTIFY darkChanged)

public:
    explicit ThemeController(QObject* parent = nullptr);

    // "system", "light" or "dark"
    [[nodiscard]] QString preference() const;
    void setPreference(const QString& preference);

    [[nodiscard]] bool dark() const { return m_dark; }

    [[nodiscard]] QColor surface() const;
    [[nodiscard]] QColor primaryInk() const;
    [[nodiscard]] QColor secondaryInk() const;
    [[nodiscard]] QColor mutedInk() const;
    [[nodiscard]] QColor gridline() const;
    [[nodiscard]] QColor axisLine() const;
    [[nodiscard]] QColor good() const;
    [[nodiscard]] QColor serious() const;
    [[nodiscard]] QColor critical() const;

signals:
    void preferenceChanged();
    void darkChanged();

private:
    void resolve(bool force = false);
    [[nodiscard]] bool systemPrefersDark() const;

    bool m_dark = false;
};
