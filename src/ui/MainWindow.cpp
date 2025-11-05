// MainWindow.cpp
#include "ui/MainWindow.h"
#include "ui_MainWindow.h"

#include "ui/mainwindow/DraggableListWidget.h"
#include "ui/widgets/ComponentLibraryWidget.h"
#include "ui/mainwindow/TabManager.h"
#include "ui/mainwindow/FileManager.h"
#include "ui/mainwindow/RecentProjectsManager.h"
#include "ui/mainwindow/WidgetManager.h"
#include "ui/mainwindow/TextItemManager.h"
#include "ui/widgets/MinimapWidget.h"

#include <QGraphicsScene>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QEvent>
#include <QTimer>
#include <QSettings>
#include <QListWidget>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ui/widgets/dragdropgraphicsview.h"
#include "ui/widgets/VerticalToolbar.h"
#include "ui/widgets/ControlButtonsWidget.h"
#include "ui/widgets/TerminalSectionWidget.h"
#include "scene/SchematicScene.h"
#include "parsers/SvParser.h"
#include "parsers/ComponentPortParser.h"
#include "graphics/ModuleGraphicsItem.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/TextGraphicsItem.h"
#include "utils/PersistenceManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_undoStack(new QUndoStack(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_tabManager(nullptr)
    , m_fileManager(nullptr)
    , m_recentProjectsManager(nullptr)
    , m_widgetManager(nullptr)
    , m_textItemManager(nullptr)
    , m_componentLibrary(nullptr)
    , m_isLoadingProject(false)
{
    ui->setupUi(this);

    // Replace componentList with professional file explorer tree widget
    m_fileExplorerTree = new FileExplorerTreeWidget();
    ui->horizontalLayout->replaceWidget(ui->componentList, m_fileExplorerTree);
    delete ui->componentList;
    ui->componentList = nullptr; // Clear the old reference
    
    // Connect file explorer signals
    connect(m_fileExplorerTree, &FileExplorerTreeWidget::fileDoubleClicked,
            this, &MainWindow::onFileExplorerFileDoubleClicked);
    connect(m_fileExplorerTree, &FileExplorerTreeWidget::directoryDoubleClicked,
            this, &MainWindow::onFileExplorerDirectoryDoubleClicked);
    connect(m_fileExplorerTree, &FileExplorerTreeWidget::fileSelected,
            this, &MainWindow::onFileExplorerFileSelected);
    connect(m_fileExplorerTree, &FileExplorerTreeWidget::contextMenuRequested,
            this, &MainWindow::onFileExplorerContextMenuRequested);

    // Replace readyComponentList with custom card-based library widget
    ComponentLibraryWidget* componentLibrary = new ComponentLibraryWidget();
    
    // Find and replace the readyComponentList widget
    QListWidget* oldReadyList = ui->readyComponentList;
    if (!oldReadyList) {
        oldReadyList = findChild<QListWidget*>("readyComponentList");
    }
    
    if (oldReadyList) {
        QLayout* parentLayout = oldReadyList->parentWidget() ? oldReadyList->parentWidget()->layout() : nullptr;
        if (parentLayout) {
            parentLayout->replaceWidget(oldReadyList, componentLibrary);
            delete oldReadyList;
        }
    }
    
    // Store reference to the new library widget
    ui->readyComponentList = nullptr; // Clear the old reference
    m_componentLibrary = componentLibrary;

    // Replace horizontal layout with splitter for resizable editor panel
    setupResizableEditorPanel();

    // Setup scene
    scene = new SchematicScene(this);
    DragDropGraphicsView* graphicsView = static_cast<DragDropGraphicsView*>(ui->graphicsView);
    graphicsView->setSharedScene(scene);
    
    // Set graphics view for drag-drop interactions if needed by widgets
    // (drag list removed; graphics view already initialized)
    
    // Setup undo/redo functionality
    setupUndoRedo();
    
    // Setup file watcher for RTL changes
    setupFileWatcher();
    
    // Setup action buttons layout (before control buttons are added)
    setupActionButtonsLayout();
    
    // Setup control buttons widget (before managers to ensure it's ready)
    setupControlButtonsWidget();
    
    // Setup all managers
    setupManagers();
    
    // Setup terminal section
    setupTerminalSection();
    
    // Setup terminal menu actions
    setupTerminalMenuActions();
    
    // Connect double-click on RTL list to open files (legacy list widget may be null now)
    if (ui->componentList) {
        connect(ui->componentList, &QListWidget::itemDoubleClicked, this, &MainWindow::onRtlListDoubleClicked);
    }

    // Try to load from current directory if top.sv exists
    QDir currentDir = QDir::current();
    if (currentDir.exists("top.sv")) {
        // Normalize the directory path
        currentRtlDirectory = currentDir.absolutePath();
        loadProjectInternal(currentRtlDirectory);
    }
}

void MainWindow::setupResizableEditorPanel()
{
    // Find the topWidget that contains the horizontal layout
    QWidget* topWidget = findChild<QWidget*>("topWidget");
    if (!topWidget) {
        qWarning() << "topWidget not found";
        return;
    }
    
    // Get the existing layout and widgets
    QHBoxLayout* oldLayout = qobject_cast<QHBoxLayout*>(topWidget->layout());
    if (!oldLayout) {
        qWarning() << "horizontalLayout not found";
        return;
    }
    
    // Find the widgets we need to preserve
    QWidget* fileExplorerContainer = nullptr;
    QWidget* tabWidget = ui->tabWidget;
    QWidget* editComponentWidget = findChild<QWidget*>("widgetEditComponent");
    
    // Find the file explorer container (vertical layout with componentList and readyComponentList)
    // Look for the first layout item (should be the file explorer vertical layout)
    for (int i = 0; i < oldLayout->count(); ++i) {
        QLayoutItem* item = oldLayout->itemAt(i);
        if (item->layout() && !fileExplorerContainer) {
            // Create a container widget for the file explorer
            fileExplorerContainer = new QWidget(topWidget);
            fileExplorerContainer->setObjectName("fileExplorerContainer");
            
            // Get the vertical layout
            QVBoxLayout* vertLayout = qobject_cast<QVBoxLayout*>(item->layout());
            if (vertLayout) {
                // Create a new layout for the container
                QVBoxLayout* newVertLayout = new QVBoxLayout(fileExplorerContainer);
                newVertLayout->setContentsMargins(0, 0, 0, 0);
                newVertLayout->setSpacing(vertLayout->spacing());
                
                // Move all widgets from old layout to new container
                QList<QWidget*> widgets;
                while (vertLayout->count() > 0) {
                    QLayoutItem* child = vertLayout->takeAt(0);
                    if (child->widget()) {
                        widgets.append(child->widget());
                    }
                    delete child;
                }
                
                // Add widgets to new layout
                for (QWidget* w : widgets) {
                    newVertLayout->addWidget(w);
                }
                
                fileExplorerContainer->setLayout(newVertLayout);
            }
            break;
        }
    }
    
    if (!fileExplorerContainer || !tabWidget || !editComponentWidget) {
        qWarning() << "Could not find all required widgets for splitter setup";
        qWarning() << "fileExplorerContainer:" << fileExplorerContainer;
        qWarning() << "tabWidget:" << tabWidget;
        qWarning() << "editComponentWidget:" << editComponentWidget;
        return;
    }
    
    // Remove widgets from old layout (but don't delete them)
    while (oldLayout->count() > 0) {
        QLayoutItem* item = oldLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->setParent(nullptr);
        }
        delete item;
    }
    
    // Delete the old layout
    delete oldLayout;
    
    // Create a new splitter
    QSplitter* mainHorizontalSplitter = new QSplitter(Qt::Horizontal, topWidget);
    mainHorizontalSplitter->setObjectName("mainHorizontalSplitter");
    mainHorizontalSplitter->setChildrenCollapsible(false);
    mainHorizontalSplitter->setHandleWidth(4);
    
    // Add widgets to splitter
    mainHorizontalSplitter->addWidget(fileExplorerContainer);
    mainHorizontalSplitter->addWidget(tabWidget);
    mainHorizontalSplitter->addWidget(editComponentWidget);
    
    // Make sure file explorer is visible
    fileExplorerContainer->show();
    
    // Set initial sizes (similar to old stretch: 1,5,1 but in pixels)
    // Assuming a window width of 880px from UI: 125px, 625px, 125px (but start with 0 for editor)
    QList<int> sizes;
    sizes << 150 << 730 << 0;  // Editor starts hidden
    mainHorizontalSplitter->setSizes(sizes);
    
    // Set stretch factors (similar to old behavior)
    mainHorizontalSplitter->setStretchFactor(0, 1);  // File explorer
    mainHorizontalSplitter->setStretchFactor(1, 5);  // Schematic
    mainHorizontalSplitter->setStretchFactor(2, 0);  // Editor (no stretch, user controlled)
    
    // Set the splitter as the topWidget's layout
    QVBoxLayout* newLayout = new QVBoxLayout(topWidget);
    newLayout->setContentsMargins(0, 0, 0, 0);
    newLayout->setSpacing(0);
    newLayout->addWidget(mainHorizontalSplitter);
    topWidget->setLayout(newLayout);
    
    qDebug() << "✅ Resizable editor panel setup complete";
    qDebug() << "   File Explorer Container:" << fileExplorerContainer;
    qDebug() << "   Splitter sizes:" << mainHorizontalSplitter->sizes();
}

void MainWindow::setupActionButtonsLayout()
{
    // Find the action_buttons_layout (it's a QHBoxLayout based on the UI file)
    QHBoxLayout* actionButtonsLayout = ui->action_buttons_layout;
    if (!actionButtonsLayout) {
        // Fallback: try to find it by name
        actionButtonsLayout = findChild<QHBoxLayout*>("action_buttons_layout");
    }
    
    if (!actionButtonsLayout) {
        qWarning() << "action_buttons_layout not found - buttons will remain in horizontalLayout_3";
        return;
    }
    
    // Find horizontalLayout_3
    QHBoxLayout* horizontalLayout = ui->horizontalLayout_3;
    if (!horizontalLayout) {
        qWarning() << "horizontalLayout_3 not found";
        return;
    }
    
    // Collect all push buttons from horizontalLayout_3
    // We need to collect them first, then remove them (to avoid index issues)
    QList<QPushButton*> actionButtons;
    QList<QLayoutItem*> itemsToRemove;
    
    for (int i = 0; i < horizontalLayout->count(); ++i) {
        QLayoutItem* item = horizontalLayout->itemAt(i);
        if (item && item->widget()) {
            QPushButton* button = qobject_cast<QPushButton*>(item->widget());
            if (button) {
                // Only collect buttons that are direct children (not part of ControlButtonsWidget)
                // Check if button's parent is centralwidget (meaning it's a direct button)
                QWidget* parent = button->parentWidget();
                if (parent && parent == ui->centralwidget) {
                    actionButtons.append(button);
                    itemsToRemove.append(item);
                }
            }
        }
    }
    
    // Remove buttons from horizontalLayout_3 and add them to action_buttons_layout
    for (QPushButton* button : actionButtons) {
        horizontalLayout->removeWidget(button);
        actionButtonsLayout->addWidget(button);
        button->show(); // Make sure buttons are visible
    }
    
    // Make sure the layout itself is visible
    if (actionButtonsLayout->parentWidget()) {
        actionButtonsLayout->parentWidget()->show();
    }
    
    qDebug() << "✅ Action buttons moved to action_buttons_layout:" << actionButtons.size() << "buttons";
    if (actionButtons.isEmpty()) {
        qWarning() << "No action buttons found in horizontalLayout_3";
        qDebug() << "   horizontalLayout_3 count:" << horizontalLayout->count();
    }
}

void MainWindow::setupControlButtonsWidget()
{
    qDebug() << "🔧 Setting up control buttons widget...";
    
    // Find horizontalLayout_2 where control buttons should be placed
    QHBoxLayout* layout = nullptr;
    
    // Try multiple ways to find the layout
    layout = findChild<QHBoxLayout*>("horizontalLayout_2");
    if (!layout) {
        layout = ui->centralwidget->findChild<QHBoxLayout*>("horizontalLayout_2");
    }
    if (!layout) {
        // Try to find it in the vertical layout structure
        QVBoxLayout* mainLayout = ui->verticalLayout_2;
        if (mainLayout) {
            for (int i = 0; i < mainLayout->count(); ++i) {
                QLayoutItem* item = mainLayout->itemAt(i);
                if (item && item->layout()) {
                    QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(item->layout());
                    if (hLayout && hLayout->objectName() == "horizontalLayout_2") {
                        layout = hLayout;
                        break;
                    }
                }
            }
        }
    }
    
    if (!layout) {
        qWarning() << "⚠️ horizontalLayout_2 not found, creating fallback container";
        
        // Fallback: Create a new layout and add it to centralwidget
        QWidget* containerWidget = new QWidget(ui->centralwidget);
        containerWidget->setObjectName("controlButtonsContainer");
        layout = new QHBoxLayout(containerWidget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(5);
        
        // Find verticalLayout_2 and add container widget
        QVBoxLayout* mainLayout = ui->verticalLayout_2;
        if (mainLayout) {
            // Insert after horizontalLayout_3 (index 1) but before mainSplitter
            mainLayout->insertWidget(1, containerWidget);
            qDebug() << "   Created fallback container and inserted at index 1";
        } else {
            qWarning() << "   Could not find verticalLayout_2 to add container";
            delete containerWidget;
            return;
        }
    }
    
    qDebug() << "   Found layout:" << layout->objectName();
    
    // Create control buttons widget
    QWidget* parentWidget = layout->parentWidget();
    if (!parentWidget) {
        parentWidget = ui->centralwidget;
    }
    
    ControlButtonsWidget* controlButtons = new ControlButtonsWidget(parentWidget);
    controlButtons->setObjectName("controlButtonsWidget");
    
    // Add spacer to push buttons to the right
    layout->addStretch();
    
    // Add the control buttons widget to horizontalLayout_2
    layout->addWidget(controlButtons);
    
    // Make sure everything is visible
    controlButtons->show();
    controlButtons->setVisible(true);
    controlButtons->raise();
    
    if (parentWidget) {
        parentWidget->show();
        parentWidget->setVisible(true);
    }
    
    if (layout->parentWidget()) {
        layout->parentWidget()->show();
        layout->parentWidget()->setVisible(true);
    }
    
    // Connect button signals
    connect(controlButtons, &ControlButtonsWidget::runClicked, this, [this]() {
        qDebug() << "▶ Run button clicked - executing make verilate in WSL";
        executeMakeVerilate();
    });
    
    connect(controlButtons, &ControlButtonsWidget::stopClicked, this, [this]() {
        qDebug() << "⏹ Stop button clicked - implement functionality";
        // TODO: Implement stop functionality
    });
    
    connect(controlButtons, &ControlButtonsWidget::pauseClicked, this, [this]() {
        qDebug() << "⏸ Pause button clicked - implement functionality";
        // TODO: Implement pause functionality
    });
    
    qDebug() << "✅ Control buttons widget created and added to layout";
    qDebug() << "   Widget size:" << controlButtons->size();
    qDebug() << "   Widget visible:" << controlButtons->isVisible();
    qDebug() << "   Layout parent:" << (layout->parentWidget() ? layout->parentWidget()->objectName() : "none");
}

void MainWindow::executeMakeVerilate()
{
    if (!m_terminalSection) {
        qWarning() << "Terminal section not available";
        return;
    }
    
    // Show terminal section and switch to terminal tab
    m_terminalSection->setVisible(true);
    m_terminalSection->showTerminalTab();
    
    // Get terminal tab and current session
    TerminalTab* terminalTab = m_terminalSection->terminalTab();
    if (!terminalTab) {
        qWarning() << "Terminal tab not available";
        return;
    }
    
    // Get or create a terminal session
    TerminalSession* session = terminalTab->currentSession();
    if (!session) {
        // Create a new session if none exists
        terminalTab->addNewSession();
        session = terminalTab->currentSession();
    }
    
    if (!session) {
        qWarning() << "Could not get or create terminal session";
        return;
    }
    
    // Get current project directory
    QString projectDir;
    if (!currentRtlDirectory.isEmpty()) {
        projectDir = currentRtlDirectory;
    } else {
        // Fallback to current working directory
        projectDir = QDir::currentPath();
    }
    
    // Convert Windows path to WSL path if needed
    QString wslPath = projectDir;
    if (QDir::separator() == '\\') {
        // Windows path - convert to WSL format (C:\Users\... -> /mnt/c/Users/...)
        QString tempPath = projectDir;
        tempPath.replace('\\', '/');
        if (tempPath.length() > 2 && tempPath.at(1) == ':') {
            // Drive letter path (C:/Users/... -> /mnt/c/Users/...)
            QChar driveLetter = tempPath.at(0).toLower();
            wslPath = QString("/mnt/%1%2").arg(driveLetter).arg(tempPath.mid(2));
        } else {
            wslPath = tempPath;
        }
    }
    
    qDebug() << "Executing make verilate in WSL";
    qDebug() << "Project directory (Windows):" << projectDir;
    qDebug() << "Project directory (WSL):" << wslPath;
    
    // Check if WSL is available
    QStringList availableShells = session->getAvailableShells();
    if (!availableShells.contains("WSL")) {
        qWarning() << "WSL not available";
        QMessageBox::warning(this, "WSL Not Available", 
                             "WSL (Windows Subsystem for Linux) is not available.\n"
                             "Please install WSL and Ubuntu to use this feature.");
        return;
    }
    
    // Clear terminal and switch to WSL
    // Clear terminal content first
    session->clearTerminal();
    
    // Switch to WSL shell (this will close old terminal and start new one)
    session->switchShell("WSL");
    
    // Wait for terminal to start, then execute command
    QTimer::singleShot(2000, [session, wslPath]() {
        if (!session->isActive()) {
            // If still not active, try starting again
            session->startTerminal();
            QTimer::singleShot(1000, [session, wslPath]() {
                // Execute cd and make verilate as a single command
                QString command = QString("cd %1 && make verilate").arg(wslPath);
                session->executeCommand(command);
            });
        } else {
            // Execute cd and make verilate as a single command
            QString command = QString("cd %1 && make verilate").arg(wslPath);
            session->executeCommand(command);
        }
    });
}

void MainWindow::setupTerminalSection()
{
    // Create terminal section widget
    m_terminalSection = new TerminalSectionWidget(this);
    m_terminalSection->setObjectName("terminalSection");
    
    // Find the main splitter (vertical splitter that contains topWidget)
    QSplitter* mainSplitter = findChild<QSplitter*>("mainSplitter");
    if (!mainSplitter) {
        qWarning() << "mainSplitter not found, cannot add terminal section";
        return;
    }
    
    // Add terminal section to the main splitter
    mainSplitter->addWidget(m_terminalSection);
    
    // Set initial sizes - make terminal section smaller initially
    QList<int> sizes = mainSplitter->sizes();
    if (sizes.size() >= 2) {
        // Adjust sizes to give more space to the top widget initially
        int totalHeight = mainSplitter->height();
        sizes[0] = static_cast<int>(totalHeight * 0.7); // 70% for schematic
        sizes.append(static_cast<int>(totalHeight * 0.3)); // 30% for terminal
        mainSplitter->setSizes(sizes);
    }
    
    // Set stretch factors
    mainSplitter->setStretchFactor(0, 1); // Schematic area
    mainSplitter->setStretchFactor(1, 0); // Terminal section (no stretch)
    
    // Make terminal section collapsible
    mainSplitter->setChildrenCollapsible(true);
    
    // Initially hide the terminal section (user can show it via menu or shortcut)
    m_terminalSection->setVisible(false);
    
    qDebug() << "✅ Terminal section setup complete";
    qDebug() << "   Terminal section:" << m_terminalSection;
    qDebug() << "   Main splitter sizes:" << mainSplitter->sizes();
}

void MainWindow::setupTerminalMenuActions()
{
    // Create Toggle Terminal action
    QAction* toggleTerminalAction = findChild<QAction*>("actionToggleTerminal");
    if (!toggleTerminalAction) {
        toggleTerminalAction = new QAction(tr("&Toggle Terminal"), this);
        toggleTerminalAction->setObjectName("actionToggleTerminal");
        toggleTerminalAction->setShortcut(QKeySequence("Ctrl+`")); // VS Code style shortcut
        toggleTerminalAction->setStatusTip(tr("Toggle Terminal Panel"));
        connect(toggleTerminalAction, &QAction::triggered, this, &MainWindow::on_actionToggleTerminal_triggered);
        
        // Add to View menu (create if doesn't exist)
        QMenu* viewMenu = nullptr;
        foreach (QAction* action, menuBar()->actions()) {
            if (action->text().contains("View")) {
                viewMenu = action->menu();
                break;
            }
        }
        if (!viewMenu) {
            viewMenu = menuBar()->addMenu(tr("&View"));
        }
        viewMenu->addAction(toggleTerminalAction);
    }
    
    // Create Problems action
    QAction* showProblemsAction = new QAction(tr("&Problems"), this);
    showProblemsAction->setObjectName("actionShowProblems");
    showProblemsAction->setShortcut(QKeySequence("Ctrl+Shift+M"));
    showProblemsAction->setStatusTip(tr("Show Problems Panel"));
    connect(showProblemsAction, &QAction::triggered, [this]() {
        if (m_terminalSection) {
            m_terminalSection->setVisible(true);
            m_terminalSection->showProblemsTab();
        }
    });
    
    // Create Output action
    QAction* showOutputAction = new QAction(tr("&Output"), this);
    showOutputAction->setObjectName("actionShowOutput");
    showOutputAction->setShortcut(QKeySequence("Ctrl+Shift+U"));
    showOutputAction->setStatusTip(tr("Show Output Panel"));
    connect(showOutputAction, &QAction::triggered, [this]() {
        if (m_terminalSection) {
            m_terminalSection->setVisible(true);
            m_terminalSection->showOutputTab();
        }
    });
    
    // Create Terminal action
    QAction* showTerminalAction = new QAction(tr("&Terminal"), this);
    showTerminalAction->setObjectName("actionShowTerminal");
    showTerminalAction->setShortcut(QKeySequence("Ctrl+Shift+`"));
    showTerminalAction->setStatusTip(tr("Show Terminal Panel"));
    connect(showTerminalAction, &QAction::triggered, [this]() {
        if (m_terminalSection) {
            m_terminalSection->setVisible(true);
            m_terminalSection->showTerminalTab();
        }
    });
    
    // Create Debug Console action
    QAction* showDebugConsoleAction = new QAction(tr("&Debug Console"), this);
    showDebugConsoleAction->setObjectName("actionShowDebugConsole");
    showDebugConsoleAction->setShortcut(QKeySequence("Ctrl+Shift+Y"));
    showDebugConsoleAction->setStatusTip(tr("Show Debug Console Panel"));
    connect(showDebugConsoleAction, &QAction::triggered, [this]() {
        if (m_terminalSection) {
            m_terminalSection->setVisible(true);
            m_terminalSection->showDebugConsoleTab();
        }
    });
    
    // Add to View menu
    QMenu* viewMenu = nullptr;
    foreach (QAction* action, menuBar()->actions()) {
        if (action->text().contains("View")) {
            viewMenu = action->menu();
            break;
        }
    }
    if (viewMenu) {
        viewMenu->addSeparator();
        viewMenu->addAction(showProblemsAction);
        viewMenu->addAction(showOutputAction);
        viewMenu->addAction(showTerminalAction);
        viewMenu->addAction(showDebugConsoleAction);
    }
    
    statusBar()->showMessage(tr("Terminal panel ready (Ctrl+` to toggle)"), 3000);
}

void MainWindow::setupManagers()
{
    // Create all managers
    m_tabManager = new TabManager(this, ui->tabWidget);
    m_fileManager = new FileManager(this, m_fileExplorerTree);
    m_recentProjectsManager = new RecentProjectsManager(this);
    m_widgetManager = new WidgetManager(this, ui->graphicsView);
    m_textItemManager = new TextItemManager(this, scene);
    
    // Setup file watcher for file manager
    m_fileManager->setFileWatcher(m_fileWatcher);
    
    // Setup tab widget
    m_tabManager->setupTabWidget();
    
    // Setup recent projects menu
    m_recentProjectsManager->setupRecentProjectsMenu();
    
    // Initialize ready components (now using card-based library)
    if (m_componentLibrary) {
        DragDropGraphicsView* graphicsView = static_cast<DragDropGraphicsView*>(ui->graphicsView);
        m_componentLibrary->setGraphicsView(graphicsView);
    }
    
    // Setup minimap
    m_widgetManager->setupMinimap();
    
    
    // Setup schematic overlays (navigation and toolbar)
    m_widgetManager->setupSchematicOverlays();
    
    // Setup edit component widget
    m_widgetManager->setupEditComponentWidget();
    
    // Connect scene context menu signals
    connect(scene, &SchematicScene::addTextRequested, m_textItemManager, &TextItemManager::onAddTextAtPosition);
    
    // Install event filter on graphics view to catch resize events
    ui->graphicsView->installEventFilter(this);
}

void MainWindow::setupUndoRedo()
{
    // Create Undo action if it doesn't exist
    QAction* undoAction = findChild<QAction*>("actionUndo");
    if (!undoAction) {
        undoAction = new QAction(tr("&Undo"), this);
        undoAction->setObjectName("actionUndo");
        undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
        undoAction->setEnabled(false);
        connect(undoAction, &QAction::triggered, this, &MainWindow::on_actionUndo_triggered);
        
        // Add to Edit menu (create if doesn't exist)
        QMenu* editMenu = nullptr;
        foreach (QAction* action, menuBar()->actions()) {
            if (action->text().contains("Edit")) {
                editMenu = action->menu();
                break;
            }
        }
        if (!editMenu) {
            editMenu = menuBar()->addMenu(tr("&Edit"));
        }
        editMenu->addAction(undoAction);
    }
    
    // Create Redo action if it doesn't exist
    QAction* redoAction = findChild<QAction*>("actionRedo");
    if (!redoAction) {
        redoAction = new QAction(tr("&Redo"), this);
        redoAction->setObjectName("actionRedo");
        redoAction->setShortcut(QKeySequence::Redo); // Ctrl+Shift+Z or Ctrl+Y
        redoAction->setEnabled(false);
        connect(redoAction, &QAction::triggered, this, &MainWindow::on_actionRedo_triggered);
        
        QMenu* editMenu = nullptr;
        foreach (QAction* action, menuBar()->actions()) {
            if (action->text().contains("Edit")) {
                editMenu = action->menu();
                break;
            }
        }
        if (editMenu) {
            editMenu->addAction(redoAction);
        }
    }
    
    // Connect undo stack signals to update action states
    connect(m_undoStack, &QUndoStack::canUndoChanged, undoAction, &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, redoAction, &QAction::setEnabled);
    
    // Update action text with command descriptions
    connect(m_undoStack, &QUndoStack::undoTextChanged, [undoAction](const QString& text) {
        undoAction->setText(text.isEmpty() ? tr("&Undo") : tr("&Undo %1").arg(text));
    });
    connect(m_undoStack, &QUndoStack::redoTextChanged, [redoAction](const QString& text) {
        redoAction->setText(text.isEmpty() ? tr("&Redo") : tr("&Redo %1").arg(text));
    });
    
    // Add keyboard shortcuts that work application-wide
    QShortcut* undoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    connect(undoShortcut, &QShortcut::activated, this, &MainWindow::on_actionUndo_triggered);
    
    QShortcut* redoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z), this);
    connect(redoShortcut, &QShortcut::activated, this, &MainWindow::on_actionRedo_triggered);
    
    // Alternative Redo shortcut (Ctrl+Y)
    QShortcut* redoShortcutAlt = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y), this);
    connect(redoShortcutAlt, &QShortcut::activated, this, &MainWindow::on_actionRedo_triggered);
    
    statusBar()->showMessage(tr("Undo/Redo ready (Ctrl+Z / Ctrl+Shift+Z / Ctrl+Y)"), 3000);
}

void MainWindow::setupFileWatcher()
{
    // Connect file watcher signal
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::onRtlFileChanged);
    
    // Create Refresh action
    QAction* refreshAction = findChild<QAction*>("actionRefresh");
    if (!refreshAction) {
        refreshAction = new QAction(tr("&Refresh"), this);
        refreshAction->setObjectName("actionRefresh");
        refreshAction->setShortcut(QKeySequence::Refresh); // F5
        connect(refreshAction, &QAction::triggered, this, &MainWindow::on_actionRefresh_triggered);
        
        // Add to File menu
        QMenu* fileMenu = nullptr;
        foreach (QAction* action, menuBar()->actions()) {
            if (action->text().contains("File")) {
                fileMenu = action->menu();
                break;
            }
        }
        if (fileMenu) {
            fileMenu->addSeparator();
            fileMenu->addAction(refreshAction);
        }
    }
    
    statusBar()->showMessage(tr("File watcher ready - changes will auto-refresh"), 2000);
}

void MainWindow::loadProjectInternal(const QString& projectPath)
{
    qDebug() << "📂 MainWindow::loadProjectInternal() called for project:" << projectPath;
    
    
    // Set working directory for persistence manager
    PersistenceManager::instance().setWorkingDirectory(projectPath);
    
    // Load RTL files
    m_fileManager->loadRtlFilesFromDirectory(projectPath);
    
    // Load persisted ready components, RTL modules, connections, and text items
    PersistenceManager::instance().loadComponentsFromDirectory(scene);
    PersistenceManager::instance().loadRTLModules(scene);
    PersistenceManager::instance().loadConnections(scene);
    
    // Load text items with explicit logging
    qDebug() << "📝 Loading text items from:" << QDir(projectPath).filePath("text_items.json");
    bool textItemsLoaded = PersistenceManager::instance().loadTextItems(scene);
    if (textItemsLoaded) {
        qDebug() << "✅ Text items loaded successfully";
        } else {
        qDebug() << "⚠️ No text items loaded or error occurred";
    }
    
    // Connect signals for loaded text items
    QList<QGraphicsItem*> items = scene->items();
    int textItemCount = 0;
    for (QGraphicsItem* item : items) {
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            m_textItemManager->connectTextItemSignals(textItem);
            textItemCount++;
        }
    }
    qDebug() << "📊 Connected signals for" << textItemCount << "text item(s)";
    
    // Update widget manager with current directory
    m_widgetManager->setCurrentRtlDirectory(projectPath);
    
    // Update recent projects list
    m_recentProjectsManager->updateRecentProjects(projectPath);
}

void MainWindow::loadProject(const QString& projectPath)
{
    qDebug() << "📂 MainWindow::loadProject() called for project:" << projectPath;
    
    // Prevent multiple simultaneous loading operations
    if (m_isLoadingProject) {
        qDebug() << "📂 Already loading a project, skipping:" << projectPath;
        statusBar()->showMessage(tr("Already loading a project, please wait..."), 2000);
        return;
    }
    
    QDir dir(projectPath);
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    qDebug() << "📁 Files in directory:" << projectPath;
    for (const QString& fileName : files) {
        qDebug() << "   -" << fileName;
    }
    if (!QDir(projectPath).exists()) {
        QMessageBox::warning(
            this,
            tr("Project Not Found"),
            tr("The project directory no longer exists:\n%1").arg(projectPath)
        );
        return;
    }
    
    // Normalize the directory path
    QString normalizedDir = QDir(projectPath).absolutePath();
    
    // Check if we're already loading the same directory to prevent double-loading
    if (currentRtlDirectory == normalizedDir) {
        qDebug() << "📂 Directory already loaded, skipping reload:" << normalizedDir;
        statusBar()->showMessage(tr("Directory already loaded: %1").arg(normalizedDir), 2000);
        return;
    }
    
    m_isLoadingProject = true;
    currentRtlDirectory = normalizedDir;
    
    // Clear the scene with proper persistence cleanup
    scene->clearSceneWithPersistenceCleanup();
    
    // Ensure scene is fully cleared before proceeding
    QApplication::processEvents();
    
    // Load the project
    loadProjectInternal(normalizedDir);
    
    m_isLoadingProject = false;
    statusBar()->showMessage(tr("Loaded project: %1").arg(normalizedDir), 3000);
}

void MainWindow::openFileInTab(const QString& filePath)
{
    qDebug() << "📂 MainWindow::openFileInTab() called for file:" << filePath;
    m_tabManager->openFileInTab(filePath);
}

void MainWindow::on_actionDark_Mode_toggled(bool checked)
{
    scene->setDarkMode(checked);
}

void MainWindow::on_actionOpen_File_triggered()
{
    // Prevent multiple simultaneous loading operations
    if (m_isLoadingProject) {
        statusBar()->showMessage(tr("Already loading a project, please wait..."), 2000);
        return;
    }
    
    QString directory = QFileDialog::getExistingDirectory(
        this,
        tr("Select RTL Directory"),
        currentRtlDirectory.isEmpty() ? QDir::homePath() : currentRtlDirectory,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!directory.isEmpty()) {
        // Normalize the directory path
        QString normalizedDir = QDir(directory).absolutePath();
        
        // Check if we're already loading the same directory to prevent double-loading
        if (currentRtlDirectory == normalizedDir) {
            statusBar()->showMessage(tr("Directory already loaded: %1").arg(normalizedDir), 2000);
            return;
        }
        
        m_isLoadingProject = true;
        currentRtlDirectory = normalizedDir;
        
        // Clear the scene with proper persistence cleanup
        scene->clearSceneWithPersistenceCleanup();
        
        // Ensure scene is fully cleared before proceeding
        // Force a small delay to ensure all cleanup operations complete
        QApplication::processEvents();
        
        // Load the project
        loadProjectInternal(normalizedDir);
        
        m_isLoadingProject = false;
        statusBar()->showMessage(tr("Loaded directory: %1").arg(normalizedDir), 3000);
    }
}

void MainWindow::on_actionUndo_triggered()
{
    if (m_undoStack->canUndo()) {
        m_undoStack->undo();
        statusBar()->showMessage(tr("Undo: %1").arg(m_undoStack->undoText()), 2000);
    }
}

void MainWindow::on_actionRedo_triggered()
{
    if (m_undoStack->canRedo()) {
        m_undoStack->redo();
        statusBar()->showMessage(tr("Redo: %1").arg(m_undoStack->redoText()), 2000);
    }
}

void MainWindow::on_actionRefresh_triggered()
{
    if (currentRtlDirectory.isEmpty()) {
        statusBar()->showMessage(tr("No directory loaded"), 2000);
        return;
    }
    
    // Reload the directory
    m_fileManager->loadRtlFilesFromDirectory(currentRtlDirectory);
    
    // Refresh all placed RTL modules on the scene
    PersistenceManager& pm = PersistenceManager::instance();
    QList<QGraphicsItem*> items = scene->items();
    
    for (QGraphicsItem* item : items) {
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module && module->isRTLView()) {
            QString moduleName = pm.getRTLModuleName(module);
            QString filePath = pm.getRTLModuleFilePath(moduleName);
            if (!filePath.isEmpty()) {
                refreshModuleView(filePath);
            }
        }
    }
    
    statusBar()->showMessage(tr("Refreshed RTL files and modules"), 2000);
}

void MainWindow::on_actionToggleTerminal_triggered()
{
    if (m_terminalSection) {
        m_terminalSection->toggleVisibility();
        QString message = m_terminalSection->isVisible() ? 
                         tr("Terminal panel shown") : 
                         tr("Terminal panel hidden");
        statusBar()->showMessage(message, 2000);
    }
}

void MainWindow::onRtlFileChanged(const QString& path)
{
    qDebug() << "File changed:" << path;
    refreshModuleView(path);
    statusBar()->showMessage(tr("RTL file updated: %1").arg(QFileInfo(path).fileName()), 3000);
}

void MainWindow::refreshModuleView(const QString& filePath)
{
    // Re-parse the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Find modules in the file
    QRegularExpression moduleRegex(R"(^\s*module\s+(\w+))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i = moduleRegex.globalMatch(content);
    
    PersistenceManager& pm = PersistenceManager::instance();
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString moduleName = match.captured(1);
        
        // Find module instances in the scene
        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem* item : items) {
            ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
            if (module && module->isRTLView()) {
                QString currentModuleName = pm.getRTLModuleName(module);
                if (currentModuleName == moduleName) {
                    // Re-parse and update the module
                    ModuleInfo updatedInfo = SvParser::parseModule(filePath, moduleName);
                    if (!updatedInfo.name.isEmpty()) {
                        QPointF currentPos = module->pos();
                        
                        // Remove old module
                        scene->removeItem(module);
                        delete module;
                        
                        // Create new module with updated info
                        ModuleGraphicsItem* newModule = new ModuleGraphicsItem(updatedInfo);
                        newModule->setPos(currentPos);
                        scene->addItem(newModule);
                        
                        // Re-register with persistence manager
                        pm.setRTLModuleName(newModule, updatedInfo.name);
                        pm.saveRTLModulePlacement(updatedInfo.name, filePath, currentPos);
                        
                        qDebug() << "Refreshed module:" << moduleName << "at" << currentPos;
                    }
                }
            }
        }
    }
}

void MainWindow::onRtlListDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    // Get file path from item data
    QString filePath = item->data(Qt::UserRole).toString();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    if (filePath.isEmpty()) return;
    
    // Check if it's a directory
    if (itemType == "directory") {
        // Navigate into directory
        currentRtlDirectory = filePath;
        
        // Only clear scene if we're changing to a different project directory
        // Check if this is a different project root (not just navigating within the same project)
        QString currentProjectRoot = PersistenceManager::instance().getWorkingDirectory();
        if (currentProjectRoot.isEmpty() || !filePath.startsWith(currentProjectRoot)) {
            // This is a new project, clear the scene
            scene->clearSceneWithPersistenceCleanup();
            
            
            // Set working directory for persistence manager
            PersistenceManager::instance().setWorkingDirectory(filePath);
            
            // Load persisted components for the new project
            PersistenceManager::instance().loadComponentsFromDirectory(scene);
            PersistenceManager::instance().loadRTLModules(scene);
            PersistenceManager::instance().loadConnections(scene);
            PersistenceManager::instance().loadTextItems(scene);
        }
        
        // Load directory contents (this updates the component list)
        m_fileManager->loadRtlFilesFromDirectory(filePath);
        
        statusBar()->showMessage(tr("Navigated to: %1").arg(filePath), 2000);
    } else {
        // Open file in tab
        openFileInTab(filePath);
        statusBar()->showMessage(tr("Opened: %1").arg(QFileInfo(filePath).fileName()), 2000);
    }
}

void MainWindow::refreshComponent(const QString& filePath)
{
    qDebug() << "🔄 Refreshing component from file:" << filePath;
    
    // Parse the component file to extract port information
    ModuleInfo moduleInfo = ComponentPortParser::parseComponentFile(filePath);
    
    if (moduleInfo.name.isEmpty()) {
        qWarning() << "Failed to parse component file:" << filePath;
        return;
    }
    
    qDebug() << "📊 Parsed component:" << moduleInfo.name 
             << "| Inputs:" << moduleInfo.inputs.size() 
             << "| Outputs:" << moduleInfo.outputs.size();
    
    // Find the component on the scene by matching the component ID
    PersistenceManager& pm = PersistenceManager::instance();
    QString workingDir = pm.getWorkingDirectory();
    QString fileName = QFileInfo(filePath).fileName();
    QString componentId = fileName.left(fileName.lastIndexOf('.'));
    
    // Iterate through items to find the matching component
    QList<QGraphicsItem*> items = scene->items();
    ReadyComponentGraphicsItem* targetComponent = nullptr;
    
    for (QGraphicsItem* item : items) {
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            QString itemId = pm.getComponentId(component);
            if (itemId == componentId) {
                targetComponent = component;
                break;
            }
        }
    }
    
    if (!targetComponent) {
        qDebug() << "⚠️ Component not found on scene for:" << componentId;
        return;
    }
    
    // Refresh the component's ports from the file
    targetComponent->refreshPortsFromFile(filePath);
    
    // Show status message with port counts
    statusBar()->showMessage(
        tr("✅ Component updated: %1 (%2 inputs, %3 outputs)")
            .arg(targetComponent->getName())
            .arg(moduleInfo.inputs.size())
            .arg(moduleInfo.outputs.size()), 
        4000
    );
    
    qDebug() << "✅ Component refreshed successfully:" << componentId;
}


void MainWindow::onAddText()
{
    m_textItemManager->onAddText();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->graphicsView) {
        if (event->type() == QEvent::Resize) {
            m_widgetManager->updateMinimapPosition();
            if (m_widgetManager->minimap()) {
                m_widgetManager->minimap()->updateViewportRect();
            }
            
            // Also update schematic overlays position (includes control buttons)
            m_widgetManager->updateSchematicOverlaysPosition();
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}

// File Explorer Tree Widget Slots
void MainWindow::onFileExplorerFileDoubleClicked(const QString& filePath)
{
    qDebug() << "File double-clicked in file explorer:" << filePath;
    openFileInTab(filePath);
    statusBar()->showMessage(tr("Opened: %1").arg(QFileInfo(filePath).fileName()), 2000);
}

void MainWindow::onFileExplorerDirectoryDoubleClicked(const QString& dirPath)
{
    qDebug() << "Directory double-clicked in file explorer:" << dirPath;
    
    // Navigate into directory
    currentRtlDirectory = dirPath;
    
    // Only clear scene if we're changing to a different project directory
    QString currentProjectRoot = PersistenceManager::instance().getWorkingDirectory();
    if (currentProjectRoot.isEmpty() || !dirPath.startsWith(currentProjectRoot)) {
        // This is a new project, clear the scene
        scene->clearSceneWithPersistenceCleanup();
        
        
        // Set working directory for persistence manager
        PersistenceManager::instance().setWorkingDirectory(dirPath);
        
        // Load persisted components for the new project
        PersistenceManager::instance().loadComponentsFromDirectory(scene);
        PersistenceManager::instance().loadRTLModules(scene);
        PersistenceManager::instance().loadConnections(scene);
        PersistenceManager::instance().loadTextItems(scene);
    }
    
    // Load directory contents (this updates the file explorer)
    m_fileManager->loadRtlFilesFromDirectory(dirPath);
    
    statusBar()->showMessage(tr("Navigated to: %1").arg(dirPath), 2000);
}

void MainWindow::onFileExplorerFileSelected(const QString& filePath)
{
    qDebug() << "File selected in file explorer:" << filePath;
    // Could show file preview or properties here
}

void MainWindow::onFileExplorerContextMenuRequested(const QString& filePath, const QPoint& globalPos)
{
    qDebug() << "Context menu requested for:" << filePath << "at position:" << globalPos;
    // Context menu is handled by the FileExplorerTreeWidget itself
}

MainWindow::~MainWindow()
{
    delete ui;
}
