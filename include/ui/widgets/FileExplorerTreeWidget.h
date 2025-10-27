#ifndef FILEEXPLORERTREEWIDGET_H
#define FILEEXPLORERTREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
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

class FileExplorerTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit FileExplorerTreeWidget(QWidget *parent = nullptr);
    
    void setRootDirectory(const QString &path);
    void refreshView();
    QString getCurrentPath() const;
    
    // Icon management
    QIcon getFileIcon(const QString &filePath) const;
    QIcon getDirectoryIcon() const;
    QIcon getSystemVerilogIcon() const;
    QIcon getVerilogIcon() const;
    QIcon getModuleIcon() const;
    
signals:
    void fileDoubleClicked(const QString &filePath);
    void directoryDoubleClicked(const QString &dirPath);
    void fileSelected(const QString &filePath);
    void contextMenuRequested(const QString &filePath, const QPoint &globalPos);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemSelectionChanged();
    void onContextMenuAction();

private:
    void setupTreeWidget();
    void createIcons();
    void setupContextMenu();
    void populateTree(const QString &path, QTreeWidgetItem *parent = nullptr);
    void addFileToTree(const QFileInfo &fileInfo, QTreeWidgetItem *parent);
    void addDirectoryToTree(const QFileInfo &dirInfo, QTreeWidgetItem *parent);
    void expandToPath(const QString &path);
    
    QString m_rootPath;
    QTreeWidgetItem *m_rootItem;
    
    // Icons
    QIcon m_directoryIcon;
    QIcon m_fileIcon;
    QIcon m_systemVerilogIcon;
    QIcon m_verilogIcon;
    QIcon m_moduleIcon;
    QIcon m_cmakeIcon;
    QIcon m_jsonIcon;
    QIcon m_markdownIcon;
    QIcon m_textIcon;
    
    // Context menu
    QMenu *m_contextMenu;
    QAction *m_openAction;
    QAction *m_openInExplorerAction;
    QAction *m_refreshAction;
    QAction *m_collapseAllAction;
    QAction *m_expandAllAction;
};

#endif // FILEEXPLORERTREEWIDGET_H
