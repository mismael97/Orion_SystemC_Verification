/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "ui/widgets/dragdropgraphicsview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionDark_Mode;
    QAction *actionOpen_File;
    QAction *actionUndo;
    QAction *actionRedo;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton;
    QPushButton *pushButton_3;
    QPushButton *pushButton_2;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_4;
    QPushButton *pushButton_9;
    QPushButton *pushButton_8;
    QPushButton *pushButton_10;
    QHBoxLayout *action_buttons_layout;
    QSpacerItem *horizontalSpacer;
    QSplitter *mainSplitter;
    QWidget *topWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QListWidget *componentList;
    QLabel *label;
    QListWidget *readyComponentList;
    QVBoxLayout *schematic_and_terminal_Layout;
    QTabWidget *tabWidget;
    QWidget *schematicTab;
    QVBoxLayout *verticalLayout_schematic;
    DragDropGraphicsView *graphicsView;
    QWidget *widgetEditComponent;
    QMenuBar *menubar;
    QMenu *menuAppearance;
    QMenu *menuFile;
    QMenu *menuEdit;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(880, 703);
        actionDark_Mode = new QAction(MainWindow);
        actionDark_Mode->setObjectName("actionDark_Mode");
        actionDark_Mode->setCheckable(true);
        actionOpen_File = new QAction(MainWindow);
        actionOpen_File->setObjectName("actionOpen_File");
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName("actionUndo");
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName("actionRedo");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");

        horizontalLayout_3->addWidget(pushButton);

        pushButton_3 = new QPushButton(centralwidget);
        pushButton_3->setObjectName("pushButton_3");

        horizontalLayout_3->addWidget(pushButton_3);

        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");

        horizontalLayout_3->addWidget(pushButton_2);

        pushButton_5 = new QPushButton(centralwidget);
        pushButton_5->setObjectName("pushButton_5");

        horizontalLayout_3->addWidget(pushButton_5);

        pushButton_6 = new QPushButton(centralwidget);
        pushButton_6->setObjectName("pushButton_6");

        horizontalLayout_3->addWidget(pushButton_6);

        pushButton_7 = new QPushButton(centralwidget);
        pushButton_7->setObjectName("pushButton_7");

        horizontalLayout_3->addWidget(pushButton_7);

        pushButton_4 = new QPushButton(centralwidget);
        pushButton_4->setObjectName("pushButton_4");

        horizontalLayout_3->addWidget(pushButton_4);

        pushButton_9 = new QPushButton(centralwidget);
        pushButton_9->setObjectName("pushButton_9");

        horizontalLayout_3->addWidget(pushButton_9);

        pushButton_8 = new QPushButton(centralwidget);
        pushButton_8->setObjectName("pushButton_8");

        horizontalLayout_3->addWidget(pushButton_8);

        pushButton_10 = new QPushButton(centralwidget);
        pushButton_10->setObjectName("pushButton_10");

        horizontalLayout_3->addWidget(pushButton_10);

        action_buttons_layout = new QHBoxLayout();
        action_buttons_layout->setObjectName("action_buttons_layout");

        horizontalLayout_3->addLayout(action_buttons_layout);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout_3);

        mainSplitter = new QSplitter(centralwidget);
        mainSplitter->setObjectName("mainSplitter");
        mainSplitter->setOrientation(Qt::Orientation::Vertical);
        mainSplitter->setChildrenCollapsible(false);
        topWidget = new QWidget(mainSplitter);
        topWidget->setObjectName("topWidget");
        horizontalLayout = new QHBoxLayout(topWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        label_2 = new QLabel(topWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        componentList = new QListWidget(topWidget);
        componentList->setObjectName("componentList");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(componentList->sizePolicy().hasHeightForWidth());
        componentList->setSizePolicy(sizePolicy);
        componentList->setMinimumSize(QSize(250, 0));

        verticalLayout->addWidget(componentList);

        label = new QLabel(topWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        readyComponentList = new QListWidget(topWidget);
        readyComponentList->setObjectName("readyComponentList");

        verticalLayout->addWidget(readyComponentList);


        horizontalLayout->addLayout(verticalLayout);

        schematic_and_terminal_Layout = new QVBoxLayout();
        schematic_and_terminal_Layout->setObjectName("schematic_and_terminal_Layout");
        tabWidget = new QTabWidget(topWidget);
        tabWidget->setObjectName("tabWidget");
        schematicTab = new QWidget();
        schematicTab->setObjectName("schematicTab");
        verticalLayout_schematic = new QVBoxLayout(schematicTab);
        verticalLayout_schematic->setObjectName("verticalLayout_schematic");
        graphicsView = new DragDropGraphicsView(schematicTab);
        graphicsView->setObjectName("graphicsView");

        verticalLayout_schematic->addWidget(graphicsView);

        tabWidget->addTab(schematicTab, QString());

        schematic_and_terminal_Layout->addWidget(tabWidget);


        horizontalLayout->addLayout(schematic_and_terminal_Layout);

        widgetEditComponent = new QWidget(topWidget);
        widgetEditComponent->setObjectName("widgetEditComponent");

        horizontalLayout->addWidget(widgetEditComponent);

        horizontalLayout->setStretch(0, 2);
        horizontalLayout->setStretch(2, 1);
        mainSplitter->addWidget(topWidget);

        verticalLayout_2->addWidget(mainSplitter);

        verticalLayout_2->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 880, 21));
        menuAppearance = new QMenu(menubar);
        menuAppearance->setObjectName("menuAppearance");
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName("menuEdit");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuAppearance->menuAction());
        menuAppearance->addAction(actionDark_Mode);
        menuFile->addAction(actionOpen_File);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "OSCV", nullptr));
        actionDark_Mode->setText(QCoreApplication::translate("MainWindow", "Dark Mode", nullptr));
        actionOpen_File->setText(QCoreApplication::translate("MainWindow", "Open RTL Directory...", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen_File->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
        actionRedo->setText(QCoreApplication::translate("MainWindow", "Redo", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "Vet", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "Code Editor", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "Random", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "RTL", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "Reports", nullptr));
        pushButton_7->setText(QCoreApplication::translate("MainWindow", "T/logs", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "Tim/cm", nullptr));
        pushButton_9->setText(QCoreApplication::translate("MainWindow", "Seq/Gen", nullptr));
        pushButton_8->setText(QCoreApplication::translate("MainWindow", "Req", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow", "Waveform", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "File Explorer", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Library", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(schematicTab), QCoreApplication::translate("MainWindow", "Schematic", nullptr));
        menuAppearance->setTitle(QCoreApplication::translate("MainWindow", "Appearance", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
