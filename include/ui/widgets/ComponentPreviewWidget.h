// ComponentPreviewWidget.h
#ifndef COMPONENTPREVIEWWIDGET_H
#define COMPONENTPREVIEWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QString>
#include <QColor>
#include <memory>

class ComponentRenderer;
class ComponentPortManager;

/**
 * @brief A widget that shows a preview of how a component will look in the schematic
 * 
 * This widget displays a component exactly as it would appear when dropped
 * into the schematic scene, including proper styling, ports, and dimensions.
 */
class ComponentPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentPreviewWidget(const QString& componentName, const QString& description = "", QWidget* parent = nullptr);
    ~ComponentPreviewWidget();

    void setComponentName(const QString& name);
    void setDescription(const QString& description);
    QString getComponentName() const { return m_componentName; }
    QString getDescription() const { return m_description; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupComponent();
    void setupVisualProperties();
    void updateComponentSize();
    QSize calculateOptimalSize() const;

    QString m_componentName;
    QString m_description;
    
    // Component rendering
    std::unique_ptr<ComponentRenderer> m_renderer;
    std::unique_ptr<ComponentPortManager> m_portManager;
    
    // Component dimensions
    qreal m_width;
    qreal m_height;
    
    // Visual properties
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_neonGlowColor;
    
    // Animation timer for smooth appearance
    QTimer* m_fadeTimer;
    int m_fadeAlpha;
    static constexpr int FADE_DURATION = 200; // ms
    static constexpr int FADE_STEPS = 20;
};

#endif // COMPONENTPREVIEWWIDGET_H
