/**
 * @file PortEditorDialog.h
 * @brief Dialog for editing module port configuration
 * 
 * This dialog allows users to customize the number of inputs and outputs
 * for a module, with real-time preview of the changes.
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef PORTEDITORDIALOG_H
#define PORTEDITORDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "parsers/SvParser.h"

class ModuleGraphicsItem;

/**
 * @class PortEditorDialog
 * @brief Dialog for editing module port configuration
 * 
 * This dialog provides a user-friendly interface for customizing
 * the number of input and output ports for a module. It includes
 * a preview of the module with the new port configuration.
 */
class PortEditorDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for PortEditorDialog
     * @param moduleInfo Current module information
     * @param parent Parent widget
     * 
     * Creates a new port editor dialog with the current module configuration.
     */
    explicit PortEditorDialog(const ModuleInfo& moduleInfo, QWidget* parent = nullptr);
    
    /**
     * @brief Get the updated module information
     * @return ModuleInfo with the new port configuration
     * 
     * Returns the module information with the updated port counts
     * as configured by the user.
     */
    ModuleInfo getUpdatedModuleInfo() const;

private slots:
    /**
     * @brief Handle input count change
     * @param count New input count
     * 
     * Updates the preview when the input count changes.
     */
    void onInputCountChanged(int count);
    
    /**
     * @brief Handle output count change
     * @param count New output count
     * 
     * Updates the preview when the output count changes.
     */
    void onOutputCountChanged(int count);
    
    /**
     * @brief Handle apply button click
     * 
     * Applies the changes and closes the dialog.
     */
    void onApplyClicked();
    
    /**
     * @brief Handle cancel button click
     * 
     * Cancels the changes and closes the dialog.
     */
    void onCancelClicked();

private:
    /**
     * @brief Setup the user interface
     * 
     * Creates and arranges all UI elements in the dialog.
     */
    void setupUI();
    
    /**
     * @brief Update the module preview
     * 
     * Updates the graphics preview to show the current port configuration.
     */
    void updatePreview();
    
    /**
     * @brief Generate port names
     * @param count Number of ports
     * @param prefix Port name prefix
     * @return QList<Port> with generated port information
     * 
     * Generates port names based on the count and prefix.
     */
    QList<Port> generatePorts(int count, const QString& prefix) const;

private:
    ModuleInfo m_originalInfo;      ///< Original module information
    ModuleInfo m_currentInfo;       ///< Current module information being edited
    
    // UI elements
    QSpinBox* m_inputCountSpinBox;  ///< Spin box for input count
    QSpinBox* m_outputCountSpinBox; ///< Spin box for output count
    QGraphicsView* m_previewView;   ///< Graphics view for module preview
    QGraphicsScene* m_previewScene; ///< Graphics scene for module preview
    QPushButton* m_applyButton;     ///< Apply button
    QPushButton* m_cancelButton;    ///< Cancel button
    
    // Constants
    static constexpr int MIN_PORTS = 0;     ///< Minimum number of ports
    static constexpr int MAX_PORTS = 20;    ///< Maximum number of ports
    static constexpr int DEFAULT_WIDTH = 1; ///< Default port width
};

#endif // PORTEDITORDIALOG_H
