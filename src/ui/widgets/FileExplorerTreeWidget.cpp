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
#include <QGradient>
#include <QPolygon>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

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
    
    // Set icon size to 24x24 to match our larger icons
    setIconSize(QSize(24, 24));
    
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
    // Directory icon - folder with folded corner
    QPixmap dirPixmap(24, 24);
    dirPixmap.fill(Qt::transparent);
    QPainter dirPainter(&dirPixmap);
    dirPainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient dirGradient(0, 0, 24, 24);
    dirGradient.setColorAt(0, QColor(255, 183, 77));
    dirGradient.setColorAt(1, QColor(255, 152, 0));
    dirPainter.setBrush(dirGradient);
    dirPainter.setPen(QPen(QColor(255, 111, 0), 1.5));
    dirPainter.drawRoundedRect(2, 5, 18, 16, 2, 2);
    QPolygon folderFold;
    folderFold << QPoint(2, 5) << QPoint(9, 5) << QPoint(9, 12) << QPoint(2, 12);
    dirPainter.setBrush(QColor(255, 167, 38, 200));
    dirPainter.setPen(QPen(QColor(255, 111, 0), 1.5));
    dirPainter.drawPolygon(folderFold);
    m_directoryIcon = QIcon(dirPixmap);
    
    // SystemVerilog icon (.sv) - red circuit/grid
    QPixmap svPixmap(24, 24);
    svPixmap.fill(Qt::transparent);
    QPainter svPainter(&svPixmap);
    svPainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient svGradient(0, 0, 24, 24);
    svGradient.setColorAt(0, QColor(244, 67, 54));
    svGradient.setColorAt(1, QColor(198, 40, 40));
    svPainter.setBrush(svGradient);
    svPainter.setPen(QPen(QColor(183, 28, 28), 2));
    svPainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    svPainter.setPen(QPen(QColor(255, 255, 255), 1.5));
    svPainter.setFont(QFont("Tajawal", 11, QFont::Bold));
    svPainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "SV");
    m_systemVerilogIcon = QIcon(svPixmap);
    
    // Verilog icon (.v) - green circuit/grid
    QPixmap vPixmap(24, 24);
    vPixmap.fill(Qt::transparent);
    QPainter vPainter(&vPixmap);
    vPainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient vGradient(0, 0, 24, 24);
    vGradient.setColorAt(0, QColor(76, 175, 80));
    vGradient.setColorAt(1, QColor(56, 142, 60));
    vPainter.setBrush(vGradient);
    vPainter.setPen(QPen(QColor(46, 125, 50), 2));
    vPainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    vPainter.setPen(QPen(QColor(255, 255, 255), 1.5));
    vPainter.setFont(QFont("Tajawal", 12, QFont::Bold));
    vPainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "V");
    m_verilogIcon = QIcon(vPixmap);
    
    // Module icon (for modules within files) - blue chip
    QPixmap modulePixmap(24, 24);
    modulePixmap.fill(Qt::transparent);
    QPainter modulePainter(&modulePixmap);
    modulePainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient moduleGradient(0, 0, 24, 24);
    moduleGradient.setColorAt(0, QColor(33, 150, 243));
    moduleGradient.setColorAt(1, QColor(21, 101, 192));
    modulePainter.setBrush(moduleGradient);
    modulePainter.setPen(QPen(QColor(13, 71, 161), 2));
    modulePainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    modulePainter.setPen(QPen(QColor(255, 255, 255), 1.5));
    modulePainter.setFont(QFont("Tajawal", 10, QFont::Bold));
    modulePainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "M");
    m_moduleIcon = QIcon(modulePixmap);
    
    // CMake icon - purple
    QPixmap cmakePixmap(24, 24);
    cmakePixmap.fill(Qt::transparent);
    QPainter cmakePainter(&cmakePixmap);
    cmakePainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient cmakeGradient(0, 0, 24, 24);
    cmakeGradient.setColorAt(0, QColor(156, 39, 176));
    cmakeGradient.setColorAt(1, QColor(123, 31, 162));
    cmakePainter.setBrush(cmakeGradient);
    cmakePainter.setPen(QPen(QColor(106, 27, 154), 2));
    cmakePainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    cmakePainter.setPen(QPen(QColor(255, 255, 255), 1.5));
    cmakePainter.setFont(QFont("Tajawal", 9, QFont::Bold));
    cmakePainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "CM");
    m_cmakeIcon = QIcon(cmakePixmap);
    
    // JSON icon - yellow
    QPixmap jsonPixmap(24, 24);
    jsonPixmap.fill(Qt::transparent);
    QPainter jsonPainter(&jsonPixmap);
    jsonPainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient jsonGradient(0, 0, 24, 24);
    jsonGradient.setColorAt(0, QColor(255, 193, 7));
    jsonGradient.setColorAt(1, QColor(245, 124, 0));
    jsonPainter.setBrush(jsonGradient);
    jsonPainter.setPen(QPen(QColor(230, 81, 0), 2));
    jsonPainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    jsonPainter.setPen(QPen(QColor(66, 66, 66), 1.5));
    jsonPainter.setFont(QFont("Tajawal", 10, QFont::Bold));
    jsonPainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "J");
    m_jsonIcon = QIcon(jsonPixmap);
    
    // Markdown icon - deep purple
    QPixmap mdPixmap(24, 24);
    mdPixmap.fill(Qt::transparent);
    QPainter mdPainter(&mdPixmap);
    mdPainter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient mdGradient(0, 0, 24, 24);
    mdGradient.setColorAt(0, QColor(126, 87, 194));
    mdGradient.setColorAt(1, QColor(103, 58, 183));
    mdPainter.setBrush(mdGradient);
    mdPainter.setPen(QPen(QColor(94, 53, 177), 2));
    mdPainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    mdPainter.setPen(QPen(QColor(255, 255, 255), 1.5));
    mdPainter.setFont(QFont("Tajawal", 9, QFont::Bold));
    mdPainter.drawText(QRect(2, 2, 20, 20), Qt::AlignCenter, "M");
    mdPainter.drawText(QRect(14, 14, 8, 8), Qt::AlignCenter, "D");
    m_markdownIcon = QIcon(mdPixmap);
    
    // Text file icon - light gray
    QPixmap textPixmap(24, 24);
    textPixmap.fill(Qt::transparent);
    QPainter textPainter(&textPixmap);
    textPainter.setRenderHint(QPainter::Antialiasing);
    textPainter.setBrush(QColor(230, 230, 230));
    textPainter.setPen(QPen(QColor(180, 180, 180), 1.5));
    textPainter.drawRoundedRect(2, 2, 20, 20, 3, 3);
    QPen linePen(QColor(140, 140, 140), 1.2);
    textPainter.setPen(linePen);
    for (int i = 0; i < 3; i++) {
        textPainter.drawLine(5, 8 + i * 3, 18, 8 + i * 3);
    }
    m_textIcon = QIcon(textPixmap);
    
    // Generic file icon - white/gray document
    QPixmap filePixmap(24, 24);
    filePixmap.fill(Qt::transparent);
    QPainter filePainter(&filePixmap);
    filePainter.setRenderHint(QPainter::Antialiasing);
    filePainter.setBrush(QColor(255, 255, 255));
    filePainter.setPen(QPen(QColor(180, 180, 180), 1.5));
    QPolygon docShape;
    docShape << QPoint(4, 2) << QPoint(18, 2) << QPoint(20, 4) << QPoint(20, 22) << QPoint(4, 22);
    filePainter.drawPolygon(docShape);
    QPolygon docCorner;
    docCorner << QPoint(4, 2) << QPoint(18, 2) << QPoint(18, 16) << QPoint(4, 16);
    filePainter.setBrush(QColor(240, 240, 240));
    filePainter.setPen(QPen(QColor(180, 180, 180), 1.5));
    filePainter.drawPolygon(docCorner);
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
    } else if (suffix == "txt" || suffix == "cpp" || suffix == "h" || suffix == "hpp" || suffix == "c") {
        return m_textIcon;
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
