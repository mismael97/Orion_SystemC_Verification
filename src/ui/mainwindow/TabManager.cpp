// TabManager.cpp
#include "ui/mainwindow/TabManager.h"
#include "ui/MainWindow.h"
#include "ui/widgets/CodeEditorWidget.h"
#include <QTabWidget>
#include <QTabBar>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>
#include <QStatusBar>
#include <QDebug>

TabManager::TabManager(MainWindow* mainWindow, QTabWidget* tabWidget)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_tabWidget(tabWidget)
{
}

void TabManager::setupTabWidget()
{
    // Enable tab closing
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    
    // Set the first tab (Schematic) to not be closable
    m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    m_tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    
    // Connect tab signals
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &TabManager::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TabManager::onTabChanged);
}

void TabManager::openFileInTab(const QString& filePath)
{
    qDebug() << "📂 TabManager::openFileInTab() called for file:" << filePath;
    // Check if file is already open
    if (m_openFileTabs.contains(filePath)) {
        // Switch to existing tab
        int tabIndex = m_openFileTabs[filePath];
        m_tabWidget->setCurrentIndex(tabIndex);
        m_mainWindow->statusBar()->showMessage(tr("Switched to: %1").arg(QFileInfo(filePath).fileName()), 2000);
        return;
    }
    
    // Create new editor widget
    CodeEditorWidget* editor = new CodeEditorWidget(filePath, m_mainWindow);
    
    // Add tab
    QString fileName = QFileInfo(filePath).fileName();
    int tabIndex = m_tabWidget->addTab(editor, fileName);
    
    // Track the tab
    m_openFileTabs[filePath] = tabIndex;
    
    // Switch to new tab
    m_tabWidget->setCurrentIndex(tabIndex);
    
    // Connect signals to update tab title on modification
    connect(editor, &CodeEditorWidget::fileModified, this, [this, filePath, fileName](bool modified) {
        if (m_openFileTabs.contains(filePath)) {
            int idx = m_openFileTabs[filePath];
            QString tabTitle = modified ? fileName + " *" : fileName;
            m_tabWidget->setTabText(idx, tabTitle);
        }
    });
    
    // Connect signal to refresh component when file is saved
    connect(editor, &CodeEditorWidget::fileSaved, this, [this, filePath]() {
        onComponentFileSaved(filePath);
    });
    
    qDebug() << "Opened file in new tab:" << filePath << "at index" << tabIndex;
}

void TabManager::onTabCloseRequested(int index)
{
    // Don't allow closing the first tab (Schematic)
    if (index == 0) {
        return;
    }
    
    // Get the widget
    QWidget* widget = m_tabWidget->widget(index);
    CodeEditorWidget* editor = qobject_cast<CodeEditorWidget*>(widget);
    
    if (editor) {
        // Check if modified
        if (editor->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                m_mainWindow,
                tr("Unsaved Changes"),
                tr("The file '%1' has unsaved changes. Do you want to save before closing?").arg(editor->getFileName()),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );
            
            if (reply == QMessageBox::Save) {
                // Save the file first then refresh component
                QString filePath = editor->getFilePath();
                editor->onSaveClicked();
                // Refresh component after saving
                onComponentFileSaved(filePath);
            } else if (reply == QMessageBox::Cancel) {
                // Don't close
                return;
            }
            // Discard: continue with closing
        }
        
        // Remove from tracking map
        QString filePath = editor->getFilePath();
        m_openFileTabs.remove(filePath);
        
        // Update indices for tabs after this one
        QMap<QString, int> updatedMap;
        for (auto it = m_openFileTabs.begin(); it != m_openFileTabs.end(); ++it) {
            int tabIdx = it.value();
            if (tabIdx > index) {
                updatedMap[it.key()] = tabIdx - 1;
            } else {
                updatedMap[it.key()] = tabIdx;
            }
        }
        m_openFileTabs = updatedMap;
    }
    
    // Remove the tab
    m_tabWidget->removeTab(index);
    
    qDebug() << "Closed tab at index" << index;
}

void TabManager::onTabChanged(int index)
{
    // Optional: Add any logic when switching tabs
    if (index >= 0) {
        QString tabTitle = m_tabWidget->tabText(index);
        m_mainWindow->statusBar()->showMessage(tr("Viewing: %1").arg(tabTitle), 1000);
    }
}

void TabManager::onComponentFileSaved(const QString& filePath)
{
    qDebug() << "Component file saved:" << filePath;
    
    // Check if this is a component .cpp file
    if (!filePath.endsWith(".cpp")) {
        return;
    }
    
    // Trigger refresh after a short delay to ensure file is fully written
    QTimer::singleShot(100, m_mainWindow, [this, filePath]() {
        m_mainWindow->refreshComponent(filePath);
    });
}


