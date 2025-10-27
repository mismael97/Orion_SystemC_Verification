# SCV Project - SystemVerilog Schematic Editor Documentation

## Project Overview

The SCV Project is a comprehensive Qt-based graphical schematic editor for SystemVerilog modules with advanced features including drag-and-drop functionality, RTL visualization, intelligent wire routing, and real-time persistence synchronization. The project implements a modular architecture that separates concerns and provides excellent maintainability and extensibility.

## Architecture Overview

The project follows a clean, modular architecture with the following main components:

```
SCV_Project/
├── src/                          # Source files (.cpp)
├── include/                      # Header files (.h)
├── resources/                    # Resource files (UI, icons)
└── build/                        # Build artifacts
```

## Module Documentation

### 1. Graphics Module (`src/graphics/`, `include/graphics/`)

The graphics module provides all visual components for the schematic editor.

#### Core Components:

**ModuleGraphicsItem** (`ModuleGraphicsItem.h/cpp`)
- **Purpose**: Graphics item for SystemVerilog RTL modules with TLM port visualization
- **Key Features**:
  - RTL view mode for compact module representation
  - TLM port visualization with input/output distinction
  - Interactive port hovering with width information
  - Module resizing with visual handles
  - Integration with wire management system
  - Persistence support for module placement
- **Methods**:
  - `setRTLView(bool enabled)`: Toggle between RTL and detailed view
  - `getInputPorts()`, `getOutputPorts()`: Get port positions
  - `getPortAt(QPointF pos, bool& isInput)`: Find port at position
  - `updateWires()`: Update wire connections

**ReadyComponentGraphicsItem** (`ReadyComponentGraphicsItem.h/cpp`)
- **Purpose**: Base graphics item for ready-made verification components with modular architecture
- **Key Features**:
  - Modular design with separated concerns
  - Port management with input/output distinction
  - Wire connection support
  - Component resizing with visual handles
  - Persistence integration
  - Signal-based property change notifications
- **Architecture**:
  - `ComponentPortManager`: Port positioning and detection
  - `ComponentWireManager`: Wire connection management
  - `ComponentResizeHandler`: Resizing operations
  - `ComponentRenderer`: Rendering/painting operations

**TextGraphicsItem** (`TextGraphicsItem.h/cpp`)
- **Purpose**: Graphics item for text annotations on the schematic
- **Key Features**:
  - Editable text with font and color customization
  - In-place editing with double-click
  - Context menu for text operations
  - Position and content persistence

#### Wire Components (`src/graphics/wire/`, `include/graphics/wire/`)

**WireGraphicsItem** (`WireGraphicsItem.h/cpp`)
- **Purpose**: Graphics item for wire connections between components
- **Key Features**:
  - Orthogonal routing with control points
  - Visual feedback during wire drawing
  - Connection validation
  - Wire styling and appearance

**WireManager** (`WireManager.h/cpp`)
- **Purpose**: Global wire manager for intelligent routing and organization
- **Key Features**:
  - Wire registration and tracking
  - Intelligent routing algorithms
  - Collision detection and avoidance
  - Wire spacing and offset management
  - Z-order management for better visibility
  - Wire bundling for parallel connections

### 2. Parsers Module (`src/parsers/`, `include/parsers/`)

The parsers module handles file parsing and data extraction.

**SvParser** (`SvParser.h/cpp`)
- **Purpose**: SystemVerilog parser for extracting module information and port definitions
- **Key Features**:
  - Parse SystemVerilog module declarations
  - Extract input and output port information
  - Handle port width specifications (e.g., [7:0], [31:0])
  - Support for complex expressions in port ranges
  - Backward compatibility with legacy parsing methods
- **Supported Port Formats**:
  - `input logic clk`
  - `output logic [7:0] data`
  - `input logic [8-1:0] address`
  - `output reg [31:0] result`
- **Methods**:
  - `parseModule(QString filePath, QString moduleName)`: Parse specific module
  - `parseTopModule()`: Parse top module (backward compatibility)

**ComponentPortParser** (`ComponentPortParser.h/cpp`)
- **Purpose**: Parser for component port configurations
- **Key Features**:
  - Parse component port definitions
  - Extract port metadata
  - Validate port configurations

### 3. Scene Module (`src/scene/`, `include/scene/`)

The scene module provides the graphics scene and interaction management.

**SchematicScene** (`SchematicScene.h/cpp`)
- **Purpose**: Custom graphics scene for schematic editing with advanced wire management
- **Key Features**:
  - Grid-based background rendering with dark/light mode support
  - Wire drawing and connection management
  - Component selection and manipulation
  - Clipboard operations (copy, cut, paste, duplicate)
  - Real-time persistence synchronization
  - Intelligent wire routing and collision detection
  - Selection rectangle for multi-item selection
- **Architecture**:
  - `WireManager`: Handles intelligent wire routing and organization
  - `SchematicPersistenceSync`: Manages real-time persistence updates
  - Component interaction: Supports both ready components and RTL modules

### 4. UI Module (`src/ui/`, `include/ui/`)

The UI module provides the user interface components and management.

**MainWindow** (`MainWindow.h/cpp`)
- **Purpose**: Main application window with modular architecture and comprehensive UI management
- **Key Features**:
  - Drag-and-drop component placement
  - Real-time file system monitoring
  - Undo/redo functionality
  - Dark mode support
  - Project management
  - Component library integration
  - Multi-tab code editing
- **Architecture**:
  - `TabManager`: Manages code editor tabs and file operations
  - `FileManager`: Handles RTL file discovery and module loading
  - `RecentProjectsManager`: Manages recent project history
  - `WidgetManager`: Manages specialized UI widgets (minimap, etc.)
  - `TextItemManager`: Handles text annotation management

#### UI Widgets (`src/ui/widgets/`, `include/ui/widgets/`)

**DragDropGraphicsView** (`dragdropgraphicsview.h/cpp`)
- **Purpose**: Custom graphics view with drag-and-drop support
- **Key Features**:
  - Drag-and-drop support for components
  - Mouse wheel zooming (20%-400%)
  - Middle-button panning
  - Double-click to reset zoom
  - Delete key to remove selected items

**CodeEditorWidget** (`CodeEditorWidget.h/cpp`)
- **Purpose**: Code editor widget for SystemVerilog files
- **Key Features**:
  - Syntax highlighting
  - Code completion
  - Error detection
  - Integration with file system

**MinimapWidget** (`MinimapWidget.h/cpp`)
- **Purpose**: Minimap for navigation in large schematics
- **Key Features**:
  - Overview of entire schematic
  - Navigation rectangle
  - Zoom level indication


### 5. Persistence Module (`src/persistence/`, `include/persistence/`)

The persistence module handles data storage and retrieval.

**PersistenceManager** (`PersistenceManager.h/cpp`)
- **Purpose**: Central persistence manager with modular architecture for schematic data management
- **Key Features**:
  - JSON-based data storage
  - Real-time synchronization
  - Component metadata management
  - Connection persistence with control points
  - RTL module integration
  - Text annotation support
  - Performance optimization with caching
  - Data validation and repair
- **Architecture**:
  - `SchematicPersistence`: Manages text items and schematic metadata
  - `ComponentPersistence`: Handles ready component storage and metadata
  - `RTLModulePersistence`: Manages RTL module placement and information
  - `ConnectionPersistence`: Handles wire connections and routing data

#### Persistence Components:

**SchematicPersistence** (`SchematicPersistence.h/cpp`)
- **Purpose**: Manages text items and schematic metadata
- **Key Features**:
  - Text item persistence
  - Schematic file management
  - Metadata storage

**ComponentPersistence** (`ComponentPersistence.h/cpp`)
- **Purpose**: Handles ready component storage and metadata
- **Key Features**:
  - Component file creation
  - Metadata management
  - Performance optimization with caching
  - Data validation and repair

**RTLModulePersistence** (`RTLModulePersistence.h/cpp`)
- **Purpose**: Manages RTL module placement and information
- **Key Features**:
  - RTL module placement persistence
  - File path management
  - Module information storage

**ConnectionPersistence** (`ConnectionPersistence.h/cpp`)
- **Purpose**: Handles wire connections and routing data
- **Key Features**:
  - Connection storage with control points
  - Routing information persistence
  - Component tracking in connections

### 6. Utils Module (`src/utils/`, `include/utils/`)

The utils module provides utility functions and helper classes.

**PersistenceManager** (see above)

## Key Features

### 1. SystemVerilog Integration
- **RTL Module Parsing**: Automatically parses SystemVerilog files to extract module information
- **Port Visualization**: Displays input/output ports with width information
- **TLM Support**: Transaction Level Modeling port visualization
- **File System Monitoring**: Real-time updates when RTL files change

### 2. Advanced Graphics
- **Drag-and-Drop**: Intuitive component placement from library
- **Wire Management**: Intelligent routing with collision detection
- **Grid System**: Snap-to-grid functionality for precise placement
- **Dark Mode**: Toggle between light and dark themes
- **Zoom and Pan**: Smooth navigation with mouse wheel and middle button

### 3. Persistence System
- **Real-time Sync**: Automatic saving of changes
- **JSON Storage**: Human-readable data format
- **Metadata Management**: Rich component information storage
- **Data Validation**: Automatic repair of corrupted data
- **Performance Optimization**: Caching for improved performance

### 4. User Interface
- **Modular Architecture**: Separated concerns for better maintainability
- **Multi-tab Editing**: Code editor with syntax highlighting
- **Minimap Navigation**: Overview of large schematics
- **Undo/Redo**: Full command history support

### 5. Component System
- **Ready Components**: Pre-built verification components
- **RTL Modules**: SystemVerilog module visualization
- **Text Annotations**: Editable text labels
- **Custom Properties**: Component-specific configuration
- **Resize Handles**: Visual component resizing

## Data Flow

1. **File Discovery**: FileManager scans RTL directory for .sv/.v files
2. **Module Parsing**: SvParser extracts module information
3. **Component Creation**: ModuleGraphicsItem creates visual representation
4. **User Interaction**: Drag-and-drop, wire connections, editing
5. **Persistence**: Real-time saving to JSON files
6. **Synchronization**: Changes propagated to all components

## Build System

The project uses CMake with Qt6 integration:

```cmake
cmake_minimum_required(VERSION 3.16)
project(SCV_Project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
```

### Dependencies
- **Qt 6.9.2** (or Qt 5.x)
- **CMake 3.16** or higher
- **C++17** compatible compiler
- **qtermwidget** (included)

## Usage Examples

### Basic Workflow
1. Launch the application
2. Open RTL Directory (File → Open RTL Directory)
3. Drag modules from the component list to the canvas
4. Connect components with wires
5. Add text annotations as needed
6. Save the project

### Advanced Features
- **RTL View Toggle**: Switch between compact and detailed module views
- **Wire Optimization**: Automatic routing optimization
- **Component Resizing**: Visual resize handles for components
- **Multi-selection**: Select multiple items with selection rectangle
- **Clipboard Operations**: Copy, cut, paste, and duplicate components

## Font Configuration

The application uses the **Tajawal** font family for all Arabic text, as specified in the user preferences. This ensures proper rendering of Arabic characters throughout the UI.

## Development Guidelines

### Adding New Components
1. **Graphics Items**: Add to `src/graphics/` and `include/graphics/`
2. **UI Widgets**: Add to `src/ui/widgets/` and `include/ui/widgets/`
3. **Parsers**: Add to `src/parsers/` and `include/parsers/`

### Include Path Convention
Use relative paths from the `include/` directory:
```cpp
#include "ui/mainwindow.h"
#include "graphics/modulegraphicsitem.h"
#include "parsers/svparser.h"
```

### Header Guards
All headers use include guards in the format:
```cpp
#ifndef CLASSNAME_H
#define CLASSNAME_H
// ... content ...
#endif // CLASSNAME_H
```

## Performance Considerations

- **Caching**: Metadata caching for improved performance
- **Lazy Loading**: Components loaded on demand
- **Batch Updates**: Grouped persistence operations
- **Memory Management**: Smart pointers for automatic cleanup
- **Optimized Rendering**: Efficient graphics operations

## Error Handling

- **File I/O**: Graceful handling of file access errors
- **Parsing Errors**: Robust SystemVerilog parsing with error recovery
- **Data Validation**: Automatic repair of corrupted persistence data
- **User Feedback**: Clear error messages and status updates

## Future Enhancements

- **Plugin System**: Extensible component architecture
- **Advanced Routing**: More sophisticated wire routing algorithms
- **Collaboration**: Multi-user editing support
- **Version Control**: Integration with Git
- **Export Options**: Multiple output formats (PDF, SVG, etc.)

## License

[Add your license information here]

## Contributors

[Add contributor information here]

---

*This documentation provides a comprehensive overview of the SCV Project architecture and functionality. For specific implementation details, refer to the individual module documentation in the source files.*
