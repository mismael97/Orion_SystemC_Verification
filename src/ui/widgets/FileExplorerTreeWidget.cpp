#include "ui/widgets/FileExplorerTreeWidget.h"
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

FileExplorerTreeWidget::FileExplorerTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_rootItem(nullptr)
    , m_contextMenu(nullptr)
{
    setupTreeWidget();
    createIcons();
    setupContextMenu();
    
    // Connect signals
    connect(this, &QTreeWidget::itemDoubleClicked, this, &FileExplorerTreeWidget::onItemDoubleClicked);
    connect(this, &QTreeWidget::itemSelectionChanged, this, &FileExplorerTreeWidget::onItemSelectionChanged);
}

void FileExplorerTreeWidget::setupTreeWidget()
{
    // Set tree widget properties
    setHeaderHidden(true);
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
    setAnimated(true);
    setSortingEnabled(false);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Set style
    setStyleSheet(
        "QTreeWidget {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 4px;"
        "    font-family: 'Tajawal', sans-serif;"
        "    font-size: 12px;"
        "}"
        "QTreeWidget::item {"
        "    padding: 4px;"
        "    border: none;"
        "    color: #212529;"
        "}"
        "QTreeWidget::item:selected {"
        "    background-color: #007bff;"
        "    color: white;"
        "}"
        "QTreeWidget::item:hover {"
        "    background-color: #e9ecef;"
        "}"
        "QTreeWidget::branch {"
        "    background: transparent;"
        "}"
        "QTreeWidget::branch:has-children:!has-siblings:closed,"
        "QTreeWidget::branch:closed:has-children:has-siblings {"
        "    image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iMTIiIHZpZXdCb3g9IjAgMCAxMiAxMiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuNSA2TDcuNSA2TDYgOC41TDQuNSA2WiIgZmlsbD0iIzY2NjY2NiIvPgo8L3N2Zz4K);"
        "}"
        "QTreeWidget::branch:open:has-children:!has-siblings,"
        "QTreeWidget::branch:open:has-children:has-siblings {"
        "    image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iMTIiIHZpZXdCb3g9IjAgMCAxMiAxMiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTYgNC41TDcuNSA2TDYuNSA3LjVMNiA0LjVaIiBmaWxsPSIjNjY2NjY2Ii8+Cjwvc3ZnPgo=);"
        "}"
    );
}

void FileExplorerTreeWidget::createIcons()
{
    // Directory icon
    QPixmap dirPixmap(16, 16);
    dirPixmap.fill(Qt::transparent);
    QPainter dirPainter(&dirPixmap);
    dirPainter.setRenderHint(QPainter::Antialiasing);
    dirPainter.setPen(QPen(QColor(255, 193, 7), 1));
    dirPainter.setBrush(QColor(255, 235, 59, 180));
    dirPainter.drawRect(2, 4, 10, 8);
    dirPainter.drawRect(2, 2, 6, 3);
    m_directoryIcon = QIcon(dirPixmap);
    
    // SystemVerilog icon (.sv)
    QPixmap svPixmap(16, 16);
    svPixmap.fill(Qt::transparent);
    QPainter svPainter(&svPixmap);
    svPainter.setRenderHint(QPainter::Antialiasing);
    svPainter.setPen(QPen(QColor(220, 53, 69), 1));
    svPainter.setBrush(QColor(220, 53, 69, 50));
    svPainter.drawRect(1, 1, 14, 14);
    svPainter.setPen(QPen(QColor(220, 53, 69), 2));
    svPainter.setFont(QFont("Tajawal", 8, QFont::Bold));
    svPainter.drawText(2, 12, "SV");
    m_systemVerilogIcon = QIcon(svPixmap);
    
    // Verilog icon (.v)
    QPixmap vPixmap(16, 16);
    vPixmap.fill(Qt::transparent);
    QPainter vPainter(&vPixmap);
    vPainter.setRenderHint(QPainter::Antialiasing);
    vPainter.setPen(QPen(QColor(40, 167, 69), 1));
    vPainter.setBrush(QColor(40, 167, 69, 50));
    vPainter.drawRect(1, 1, 14, 14);
    vPainter.setPen(QPen(QColor(40, 167, 69), 2));
    vPainter.setFont(QFont("Tajawal", 8, QFont::Bold));
    vPainter.drawText(5, 12, "V");
    m_verilogIcon = QIcon(vPixmap);
    
    // Module icon (for modules within files)
    QPixmap modulePixmap(16, 16);
    modulePixmap.fill(Qt::transparent);
    QPainter modulePainter(&modulePixmap);
    modulePainter.setRenderHint(QPainter::Antialiasing);
    modulePainter.setPen(QPen(QColor(0, 123, 255), 1));
    modulePainter.setBrush(QColor(0, 123, 255, 50));
    modulePainter.drawRect(1, 1, 14, 14);
    modulePainter.setPen(QPen(QColor(0, 123, 255), 2));
    modulePainter.setFont(QFont("Tajawal", 6, QFont::Bold));
    modulePainter.drawText(2, 11, "M");
    m_moduleIcon = QIcon(modulePixmap);
    
    // CMake icon
    QPixmap cmakePixmap(16, 16);
    cmakePixmap.fill(Qt::transparent);
    QPainter cmakePainter(&cmakePixmap);
    cmakePainter.setRenderHint(QPainter::Antialiasing);
    cmakePainter.setPen(QPen(QColor(108, 117, 125), 1));
    cmakePainter.setBrush(QColor(108, 117, 125, 50));
    cmakePainter.drawRect(1, 1, 14, 14);
    cmakePainter.setPen(QPen(QColor(108, 117, 125), 2));
    cmakePainter.setFont(QFont("Tajawal", 6, QFont::Bold));
    cmakePainter.drawText(2, 11, "CM");
    m_cmakeIcon = QIcon(cmakePixmap);
    
    // JSON icon
    QPixmap jsonPixmap(16, 16);
    jsonPixmap.fill(Qt::transparent);
    QPainter jsonPainter(&jsonPixmap);
    jsonPainter.setRenderHint(QPainter::Antialiasing);
    jsonPainter.setPen(QPen(QColor(255, 193, 7), 1));
    jsonPainter.setBrush(QColor(255, 193, 7, 50));
    jsonPainter.drawRect(1, 1, 14, 14);
    jsonPainter.setPen(QPen(QColor(255, 193, 7), 2));
    jsonPainter.setFont(QFont("Tajawal", 6, QFont::Bold));
    jsonPainter.drawText(2, 11, "J");
    m_jsonIcon = QIcon(jsonPixmap);
    
    // Markdown icon
    QPixmap mdPixmap(16, 16);
    mdPixmap.fill(Qt::transparent);
    QPainter mdPainter(&mdPixmap);
    mdPainter.setRenderHint(QPainter::Antialiasing);
    mdPainter.setPen(QPen(QColor(111, 66, 193), 1));
    mdPainter.setBrush(QColor(111, 66, 193, 50));
    mdPainter.drawRect(1, 1, 14, 14);
    mdPainter.setPen(QPen(QColor(111, 66, 193), 2));
    mdPainter.setFont(QFont("Tajawal", 6, QFont::Bold));
    mdPainter.drawText(2, 11, "MD");
    m_markdownIcon = QIcon(mdPixmap);
    
    // Generic file icon
    QPixmap filePixmap(16, 16);
    filePixmap.fill(Qt::transparent);
    QPainter filePainter(&filePixmap);
    filePainter.setRenderHint(QPainter::Antialiasing);
    filePainter.setPen(QPen(QColor(108, 117, 125), 1));
    filePainter.setBrush(QColor(248, 249, 250));
    filePainter.drawRect(2, 2, 12, 12);
    m_fileIcon = QIcon(filePixmap);
}

void FileExplorerTreeWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_openAction = new QAction("Open", this);
    m_openAction->setIcon(QIcon(":/icons/open.png"));
    connect(m_openAction, &QAction::triggered, this, &FileExplorerTreeWidget::onContextMenuAction);
    
    m_openInExplorerAction = new QAction("Open in Explorer", this);
    m_openInExplorerAction->setIcon(QIcon(":/icons/explorer.png"));
    connect(m_openInExplorerAction, &QAction::triggered, this, &FileExplorerTreeWidget::onContextMenuAction);
    
    m_refreshAction = new QAction("Refresh", this);
    m_refreshAction->setIcon(QIcon(":/icons/refresh.png"));
    connect(m_refreshAction, &QAction::triggered, this, &FileExplorerTreeWidget::refreshView);
    
    m_collapseAllAction = new QAction("Collapse All", this);
    m_collapseAllAction->setIcon(QIcon(":/icons/collapse.png"));
    connect(m_collapseAllAction, &QAction::triggered, this, [this]() { collapseAll(); });
    
    m_expandAllAction = new QAction("Expand All", this);
    m_expandAllAction->setIcon(QIcon(":/icons/expand.png"));
    connect(m_expandAllAction, &QAction::triggered, this, [this]() { expandAll(); });
    
    m_contextMenu->addAction(m_openAction);
    m_contextMenu->addAction(m_openInExplorerAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_refreshAction);
    m_contextMenu->addAction(m_collapseAllAction);
    m_contextMenu->addAction(m_expandAllAction);
}

void FileExplorerTreeWidget::setRootDirectory(const QString &path)
{
    m_rootPath = path;
    clear();
    
    if (path.isEmpty() || !QDir(path).exists()) {
        return;
    }
    
    // Create root item
    QFileInfo rootInfo(path);
    m_rootItem = new QTreeWidgetItem(this);
    m_rootItem->setText(0, rootInfo.fileName().isEmpty() ? path : rootInfo.fileName());
    m_rootItem->setIcon(0, m_directoryIcon);
    m_rootItem->setData(0, Qt::UserRole, path);
    m_rootItem->setData(0, Qt::UserRole + 1, "directory");
    
    // Populate the tree
    populateTree(path, m_rootItem);
    
    // Expand root by default
    m_rootItem->setExpanded(true);
}

void FileExplorerTreeWidget::populateTree(const QString &path, QTreeWidgetItem *parent)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }
    
    // Get all entries, excluding hidden files and system files
    QFileInfoList entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks,
        QDir::DirsFirst | QDir::Name
    );
    
    // Files to exclude
    QStringList excludedFiles = {
        "connections.json",
        "rtl_placements.json",
        ".git",
        ".vs",
        "build",
        "Debug",
        "Release"
    };
    
    for (const QFileInfo &entry : entries) {
        QString fileName = entry.fileName();
        
        // Skip excluded files and directories
        if (excludedFiles.contains(fileName) || 
            fileName.startsWith('.') || 
            fileName.startsWith('_')) {
            continue;
        }
        
        if (entry.isDir()) {
            addDirectoryToTree(entry, parent);
        } else {
            addFileToTree(entry, parent);
        }
    }
}

void FileExplorerTreeWidget::addDirectoryToTree(const QFileInfo &dirInfo, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, dirInfo.fileName());
    item->setIcon(0, m_directoryIcon);
    item->setData(0, Qt::UserRole, dirInfo.absoluteFilePath());
    item->setData(0, Qt::UserRole + 1, "directory");
    
    // Add a placeholder child to show the expand arrow
    QTreeWidgetItem *placeholder = new QTreeWidgetItem(item);
    placeholder->setText(0, "...");
    placeholder->setData(0, Qt::UserRole, "placeholder");
}

void FileExplorerTreeWidget::addFileToTree(const QFileInfo &fileInfo, QTreeWidgetItem *parent)
{
    QString fileName = fileInfo.fileName();
    QString filePath = fileInfo.absoluteFilePath();
    
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, fileName);
    item->setIcon(0, getFileIcon(filePath));
    item->setData(0, Qt::UserRole, filePath);
    item->setData(0, Qt::UserRole + 1, "file");
    
    // For SystemVerilog/Verilog files, try to parse modules
    if (fileName.endsWith(".sv") || fileName.endsWith(".v")) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            
            // Find all module declarations
            QRegularExpression moduleRegex(R"(^\s*module\s+(\w+))", QRegularExpression::MultilineOption);
            QRegularExpressionMatchIterator i = moduleRegex.globalMatch(content);
            
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString moduleName = match.captured(1);
                if (!moduleName.isEmpty()) {
                    QTreeWidgetItem *moduleItem = new QTreeWidgetItem(item);
                    moduleItem->setText(0, moduleName);
                    moduleItem->setIcon(0, m_moduleIcon);
                    moduleItem->setData(0, Qt::UserRole, filePath);
                    moduleItem->setData(0, Qt::UserRole + 1, "module");
                    moduleItem->setData(0, Qt::UserRole + 2, moduleName);
                }
            }
        }
    }
}

QIcon FileExplorerTreeWidget::getFileIcon(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "sv") {
        return m_systemVerilogIcon;
    } else if (suffix == "v") {
        return m_verilogIcon;
    } else if (suffix == "cmake" || fileInfo.fileName() == "CMakeLists.txt") {
        return m_cmakeIcon;
    } else if (suffix == "json") {
        return m_jsonIcon;
    } else if (suffix == "md") {
        return m_markdownIcon;
    } else {
        return m_fileIcon;
    }
}

QIcon FileExplorerTreeWidget::getDirectoryIcon() const
{
    return m_directoryIcon;
}

QIcon FileExplorerTreeWidget::getSystemVerilogIcon() const
{
    return m_systemVerilogIcon;
}

QIcon FileExplorerTreeWidget::getVerilogIcon() const
{
    return m_verilogIcon;
}

QIcon FileExplorerTreeWidget::getModuleIcon() const
{
    return m_moduleIcon;
}

void FileExplorerTreeWidget::refreshView()
{
    if (!m_rootPath.isEmpty()) {
        setRootDirectory(m_rootPath);
    }
}

QString FileExplorerTreeWidget::getCurrentPath() const
{
    return m_rootPath;
}

void FileExplorerTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *item = itemAt(event->pos());
    if (item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        emit contextMenuRequested(filePath, event->globalPos());
        m_contextMenu->exec(event->globalPos());
    }
}

void FileExplorerTreeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTreeWidgetItem *item = itemAt(event->pos());
    if (item) {
        onItemDoubleClicked(item, 0);
    } else {
        QTreeWidget::mouseDoubleClickEvent(event);
    }
}

void FileExplorerTreeWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTreeWidgetItem *item = currentItem();
        if (item) {
            onItemDoubleClicked(item, 0);
        }
    } else {
        QTreeWidget::keyPressEvent(event);
    }
}

void FileExplorerTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (!item) return;
    
    QString filePath = item->data(0, Qt::UserRole).toString();
    QString itemType = item->data(0, Qt::UserRole + 1).toString();
    
    if (filePath.isEmpty()) return;
    
    if (itemType == "directory") {
        // Handle directory expansion/collapse
        if (item->childCount() == 1 && 
            item->child(0)->data(0, Qt::UserRole).toString() == "placeholder") {
            // Remove placeholder and populate with actual content
            delete item->child(0);
            populateTree(filePath, item);
        }
        
        item->setExpanded(!item->isExpanded());
        emit directoryDoubleClicked(filePath);
    } else if (itemType == "file" || itemType == "module") {
        emit fileDoubleClicked(filePath);
    }
}

void FileExplorerTreeWidget::onItemSelectionChanged()
{
    QTreeWidgetItem *item = currentItem();
    if (item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        emit fileSelected(filePath);
    }
}

void FileExplorerTreeWidget::onContextMenuAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    QTreeWidgetItem *item = currentItem();
    if (!item) return;
    
    QString filePath = item->data(0, Qt::UserRole).toString();
    
    if (action == m_openAction) {
        emit fileDoubleClicked(filePath);
    } else if (action == m_openInExplorerAction) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
    }
}

void FileExplorerTreeWidget::expandToPath(const QString &path)
{
    // This method would expand the tree to show a specific path
    // Implementation would traverse the tree and expand necessary items
    Q_UNUSED(path)
    // TODO: Implement path expansion logic
}
