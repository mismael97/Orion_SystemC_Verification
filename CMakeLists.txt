cmake_minimum_required(VERSION 3.16)

project(SCV_Project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set UI files directory
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/resources)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Note: qtermwidget requires external dependencies, using simple QTextEdit-based terminal instead

# Source files organized by category
set(PROJECT_SOURCES
    # Main entry point
    src/main.cpp
    
    # Resources
    resources/resources.qrc
    
    # UI components
    src/ui/MainWindow.cpp
    include/ui/MainWindow.h
    resources/MainWindow.ui
    
    # MainWindow managers (modular architecture in ui/mainwindow/)
    src/ui/mainwindow/DraggableListWidget.cpp
    include/ui/mainwindow/DraggableListWidget.h
    src/ui/mainwindow/TabManager.cpp
    include/ui/mainwindow/TabManager.h
    src/ui/mainwindow/FileManager.cpp
    include/ui/mainwindow/FileManager.h
    src/ui/mainwindow/RecentProjectsManager.cpp
    include/ui/mainwindow/RecentProjectsManager.h
    src/ui/mainwindow/WidgetManager.cpp
    include/ui/mainwindow/WidgetManager.h
    src/ui/mainwindow/TextItemManager.cpp
    include/ui/mainwindow/TextItemManager.h
    
    # UI Widgets
    src/ui/widgets/DragDropGraphicsView.cpp
    include/ui/widgets/dragdropgraphicsview.h
    src/ui/widgets/CodeEditorDialog.cpp
    include/ui/widgets/CodeEditorDialog.h
    src/ui/widgets/CodeEditorWidget.cpp
    include/ui/widgets/CodeEditorWidget.h
    src/ui/widgets/MinimapWidget.cpp
    include/ui/widgets/MinimapWidget.h
    src/ui/widgets/VerticalToolbar.cpp
    include/ui/widgets/VerticalToolbar.h
    src/ui/widgets/ControlButtonsWidget.cpp
    include/ui/widgets/ControlButtonsWidget.h
    src/ui/widgets/EditComponentWidget.cpp
    include/ui/widgets/EditComponentWidget.h
    src/ui/widgets/ComponentLibraryWidget.cpp
    include/ui/widgets/ComponentLibraryWidget.h
    src/ui/widgets/ComponentCardWidget.cpp
    include/ui/widgets/ComponentCardWidget.h
    src/ui/widgets/ComponentPreviewWidget.cpp
    include/ui/widgets/ComponentPreviewWidget.h
    src/ui/widgets/FileExplorerTreeWidget.cpp
    include/ui/widgets/FileExplorerTreeWidget.h
    src/ui/widgets/TerminalSectionWidget.cpp
    include/ui/widgets/TerminalSectionWidget.h
    # Component metadata editing widgets
    src/ui/widgets/ComponentMetadataEditor.cpp
    include/ui/widgets/ComponentMetadataEditor.h
    src/ui/widgets/ComponentPropertiesDialog.cpp
    include/ui/widgets/ComponentPropertiesDialog.h
    src/ui/widgets/PortEditorDialog.cpp
    include/ui/widgets/PortEditorDialog.h
    
    # Scene components
    src/scene/SchematicScene.cpp
    include/scene/SchematicScene.h
    src/scene/WireManager.cpp
    include/scene/WireManager.h
    
    # Graphics items
    src/graphics/ModuleGraphicsItem.cpp
    include/graphics/ModuleGraphicsItem.h
    src/graphics/ReadyComponentGraphicsItem.cpp
    include/graphics/ReadyComponentGraphicsItem.h
    src/graphics/TextGraphicsItem.cpp
    include/graphics/TextGraphicsItem.h
    
    # Ready component modules (modular architecture in graphics/ready/)
    src/graphics/ready/ComponentPortManager.cpp
    include/graphics/ready/ComponentPortManager.h
    src/graphics/ready/ComponentWireManager.cpp
    include/graphics/ready/ComponentWireManager.h
    src/graphics/ready/ComponentResizeHandler.cpp
    include/graphics/ready/ComponentResizeHandler.h
    src/graphics/ready/ComponentRenderer.cpp
    include/graphics/ready/ComponentRenderer.h
    
    # Wire components (refactored into separate modules)
    src/graphics/wire/WireGraphicsItem.cpp
    include/graphics/wire/WireGraphicsItem.h
    src/graphics/wire/WirePathBuilder.cpp
    include/graphics/wire/WirePathBuilder.h
    src/graphics/wire/WireControlPoints.cpp
    include/graphics/wire/WireControlPoints.h
    src/graphics/wire/WireRenderer.cpp
    include/graphics/wire/WireRenderer.h
    src/graphics/wire/WireSegments.cpp
    include/graphics/wire/WireSegments.h
    
    # Parsers
    src/parsers/SvParser.cpp
    include/parsers/SvParser.h
    src/parsers/ComponentPortParser.cpp
    include/parsers/ComponentPortParser.h
    
    # Utils
    src/utils/PersistenceManager.cpp
    include/utils/PersistenceManager.h
    
    # Persistence modules (refactored into separate components)
    src/persistence/SchematicPersistence.cpp
    include/persistence/SchematicPersistence.h
    src/persistence/ComponentPersistence.cpp
    include/persistence/ComponentPersistence.h
    src/persistence/RTLModulePersistence.cpp
    include/persistence/RTLModulePersistence.h
    src/persistence/ConnectionPersistence.cpp
    include/persistence/ConnectionPersistence.h
)

qt_add_executable(SCV_Project
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Add include directories
target_include_directories(SCV_Project PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SCV_Project PRIVATE Qt6::Widgets)

set_target_properties(SCV_Project PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(SCV_Project)
