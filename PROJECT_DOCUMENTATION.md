# Advanced Data Structure Visualizer - Complete Project Documentation

## 📋 Table of Contents
1. [Project Overview](#project-overview)
2. [Architecture & Design](#architecture--design)
3. [Data Structures Implemented](#data-structures-implemented)
4. [Features & Functionality](#features--functionality)
5. [File Structure](#file-structure)
6. [Navigation Flow](#navigation-flow)
7. [Technical Implementation Details](#technical-implementation-details)
8. [Build & Setup](#build--setup)
9. [Usage Guide](#usage-guide)

---

## 🎯 Project Overview

**Advanced Data Structure Visualizer** is an interactive educational desktop application built with Qt (C++) that helps students and developers understand complex data structures through beautiful visualizations and step-by-step animations.

### Key Features
- **Interactive Visualizations**: Real-time visual representation of data structure operations
- **Step-by-Step Animations**: Animated demonstrations of algorithms (BFS, DFS, tree traversals)
- **Theory Pages**: Comprehensive educational content for each data structure
- **Operation History**: Logging system to track all operations performed
- **Modern UI**: Clean, responsive interface with smooth animations

---

## 🏗️ Architecture & Design

### Technology Stack
- **Framework**: Qt 6/5 (C++)
- **Build System**: CMake
- **Language**: C++17
- **UI Framework**: Qt Widgets

### Design Patterns
- **Page-Based Navigation**: Uses `QStackedWidget` for multi-page navigation
- **Signal-Slot Architecture**: Qt's event-driven communication
- **Observer Pattern**: Signal-slot connections for UI updates
- **Separation of Concerns**: Each data structure has its own visualization class

### Application Structure
```
Application Flow:
HomePage → MenuPage → TheoryPage → OperationPage → VisualizationPage
```

---

## 📊 Data Structures Implemented

### 1. **Binary Tree**
- **Operations**: Insertion, Deletion, Search, Traversal
- **Visualization**: Hierarchical tree display with node highlighting
- **Features**: 
  - Animated insertion with path highlighting
  - Real-time tree balancing visualization
  - Step-by-step traversal animations

### 2. **Red-Black Tree**
- **Operations**: Insertion, Deletion, Search
- **Visualization**: Color-coded nodes (Red/Black) with rotation animations
- **Features**:
  - Self-balancing visualization
  - Rotation animations (left/right)
  - Algorithm explanation pages
  - Operation history logging

### 3. **Graph**
- **Operations**: Add/Remove Vertex, Add/Remove Edge, BFS, DFS
- **Visualization**: Circular node layout with edge connections
- **Features**:
  - Interactive vertex/edge management
  - Animated BFS/DFS traversals
  - Custom or auto-generated vertex IDs
  - Operation history logging
  - Responsive layout (70/30 split: graph/history)

### 4. **Hash Table**
- **Operations**: Insert, Delete, Search
- **Visualization**: Bucket-based display with collision handling
- **Features**:
  - Hash function visualization
  - Collision resolution display
  - Load factor tracking

---

## ✨ Features & Functionality

### Core Features

#### 1. **Theory Pages**
Each data structure has a dedicated theory page with:
- **Definition**: What the data structure is
- **Time Complexity**: Analysis of operations
- **Applications**: Real-world use cases
- **Advantages**: Benefits of using the structure
- **Disadvantages**: Limitations and trade-offs

#### 2. **Visualization Pages**
Interactive pages where users can:
- Perform operations (insert, delete, search, etc.)
- Watch step-by-step animations
- See real-time updates
- Track operation history

#### 3. **Logging System**
- **Red-Black Tree**: Tracks all insertions, deletions, searches
- **Graph**: Logs vertex/edge operations, BFS/DFS traversals
- **Format**: `[HH:mm:ss] OPERATION (values): description`
- **Features**: Auto-scroll, color-coded entries, persistent during session

#### 4. **Responsive UI**
- Adapts to window resizing (minimize/maximize)
- Dynamic node positioning
- Flexible layout management
- Minimum size constraints for usability

---

## 📁 File Structure

### Core Application Files
```
Advanced-DS-Visualizer/
├── main.cpp                    # Application entry point, navigation logic
├── homepage.h/cpp              # Welcome/home screen
├── menupage.h/cpp              # Data structure selection menu
├── theorypage.h/cpp            # Educational content pages
├── operationpage.h/cpp         # Operation selection pages
│
├── treeinsertion.h/cpp         # Binary Tree insertion visualization
├── treedeletion.h/cpp           # Binary Tree deletion visualization
├── redblacktree.h/cpp          # Red-Black Tree visualization + logging
├── graphvisualization.h/cpp     # Graph visualization + logging
├── hashmapvisualization.h/cpp  # Hash Table visualization
│
├── CMakeLists.txt              # Build configuration
└── PROJECT_DOCUMENTATION.md    # This file
```

### Key Components

#### `main.cpp`
- Manages `QStackedWidget` for page navigation
- Handles page creation and destruction
- Connects signals between pages
- Implements navigation flow logic

#### `theorypage.cpp`
- Dynamic content loading based on data structure
- Scrollable information cards
- "Try It Yourself" button navigation

#### `graphvisualization.cpp`
- Graph data structure implementation (adjacency list)
- BFS/DFS algorithm implementations
- Circular node layout algorithm
- Operation history logging
- Responsive UI layout

---

## 🧭 Navigation Flow

### Standard Flow (Binary Tree, Red-Black Tree, Hash Table)
```
HomePage
  ↓ (Get Started)
MenuPage
  ↓ (Select Data Structure)
TheoryPage
  ↓ (Try It Yourself)
OperationPage
  ↓ (Select Operation)
VisualizationPage
  ↓ (Back)
OperationPage
  ↓ (Back)
TheoryPage
  ↓ (Back)
MenuPage
```

### Graph Flow (Direct Access)
```
MenuPage
  ↓ (Select Graph)
TheoryPage
  ↓ (Try It Yourself)
OperationPage
  ↓ (Any Operation)
GraphVisualization (Comprehensive - all operations in one page)
  ↓ (Back)
OperationPage
```

**Note**: Graph visualization includes all operations (add vertex/edge, remove, BFS/DFS) in a single comprehensive page, so any operation selection opens the same page.

---

## 🔧 Technical Implementation Details

### Graph Visualization

#### Data Structure
- **Adjacency List**: `QHash<int, QSet<int>>` for efficient edge operations
- **Node Storage**: `QVector<GraphNode>` with position tracking
- **Undirected Graph**: Edges are bidirectional

#### Algorithms

**BFS (Breadth-First Search)**
```cpp
1. Start at selected vertex
2. Use queue (FIFO)
3. Visit level by level
4. Mark visited nodes
5. Animate each step (700ms delay)
```

**DFS (Depth-First Search)**
```cpp
1. Start at selected vertex
2. Use stack (LIFO)
3. Go deep first, then backtrack
4. Mark visited nodes
5. Animate each step (700ms delay)
```

#### Layout Algorithm
- **Circular Layout**: Nodes arranged in a circle
- **Formula**: `angle = (2π × index) / total_nodes`
- **Dynamic Sizing**: Adjusts based on canvas size
- **Minimum Radius**: 50 pixels

#### Vertex ID Generation
- **Auto-Generation**: If input empty → uses `nextId++` (0, 1, 2, ...)
- **Custom ID**: If input provided → uses that ID, validates uniqueness
- **Smart Increment**: Updates `nextId` if custom ID is higher

### Logging System

#### HistoryEntry Structure
```cpp
struct GraphHistoryEntry {
    QString operation;      // ADD_VERTEX, ADD_EDGE, BFS, etc.
    int value1;             // First value (vertex ID, etc.)
    int value2;             // Second value (for edges: -1 if not used)
    QString description;    // Human-readable description
    QString timestamp;      // HH:mm:ss format
};
```

#### Logged Operations
- `ADD_VERTEX`: Vertex additions
- `ADD_EDGE`: Edge additions (with both vertex IDs)
- `REMOVE_VERTEX`: Vertex removals
- `REMOVE_EDGE`: Edge removals
- `BFS`: Breadth-first search start and completion
- `DFS`: Depth-first search start and completion
- `CLEAR`: Graph clearing

### UI Responsiveness

#### Layout Management
- **Controls**: Maximum width constraint (1200px) prevents over-stretching
- **History Panel**: Minimum width (250px) ensures readability
- **Graph Canvas**: 70% width, dynamically calculated
- **Resize Handling**: `resizeEvent()` recalculates node positions

#### Button Sizing
- All buttons use `setMinimumSize()` and `setMaximumSize()` for consistency
- Combo boxes: Fixed 90px width
- Input fields: Fixed 120px width
- Buttons: Varying widths (120-150px) based on text

---

## 🛠️ Build & Setup

### Prerequisites
- Qt 6 or Qt 5 (Widgets module)
- CMake 3.16 or higher
- C++17 compatible compiler
- Qt Creator (recommended) or any IDE with CMake support

### Build Instructions

#### Using Qt Creator
1. Open `CMakeLists.txt` in Qt Creator
2. Configure project (select Qt version)
3. Build project (Ctrl+B)
4. Run (Ctrl+R)

#### Using Command Line
```bash
mkdir build
cd build
cmake ..
cmake --build .
./AdvDS  # or AdvDS.exe on Windows
```

### CMake Configuration
- **Minimum CMake**: 3.16
- **C++ Standard**: C++17
- **Qt Components**: Widgets
- **Auto MOC/UIC/RCC**: Enabled

---

## 📖 Usage Guide

### Getting Started

1. **Launch Application**
   - Run the executable
   - You'll see the welcome/home page

2. **Navigate to Data Structure**
   - Click "Get Started"
   - Select a data structure from the menu

3. **Learn Theory**
   - Read the theory page for definitions, complexity, applications
   - Click "Try It Yourself" to proceed

4. **Select Operation**
   - Choose an operation (Insertion, Deletion, Search, Traversal)
   - For Graph: Any operation opens the comprehensive visualization

5. **Interact with Visualization**
   - Perform operations using the controls
   - Watch animations and see real-time updates
   - Check operation history (for Graph and Red-Black Tree)

### Graph Visualization Guide

#### Adding Vertices
- **Auto-ID**: Leave input empty, click "Add Vertex" → IDs: 0, 1, 2, ...
- **Custom ID**: Type a number, click "Add Vertex" → Uses that ID

#### Adding Edges
- Select "From" vertex from dropdown
- Select "To" vertex from dropdown
- Click "Add Edge"
- Edge appears as purple line connecting nodes

#### Running Traversals
- Select start vertex from "Traversal" dropdown
- Click "Run BFS" or "Run DFS"
- Watch step-by-step animation:
  - **Orange**: Currently visiting
  - **Green**: Already visited
  - **Purple**: Not yet visited

#### Operation History
- Right panel shows all operations
- Format: `[Time] OPERATION (values): description`
- Auto-scrolls to latest entry
- Cleared when graph is cleared

---

## 🎨 UI Theme & Styling

### Color Scheme
- **Primary Purple**: `#7b4fff` (buttons, highlights)
- **Dark Purple**: `#2d1b69` (text, titles)
- **Light Purple**: `#6b5b95` (subtitles, labels)
- **Background**: Gradient from `#faf7ff` to `#ede4ff`
- **Accent**: `#9b6fff` (button gradients)

### Typography
- **Primary Font**: Segoe UI (fallback: Poppins, Arial)
- **Title Size**: 28-36px, Bold
- **Body Size**: 11-13px
- **Button Font**: 13px, Bold

### Component Styling
- **Buttons**: Rounded (19-22px radius), gradient backgrounds
- **Input Fields**: Rounded (19px radius), white background
- **Cards**: White background, rounded (16-24px radius), subtle shadows
- **History Panel**: White background, scrollable list

---

## 🔄 Animation System

### Graph Traversals
- **Pre-computation**: Algorithm runs completely to determine visit order
- **Step Animation**: 700ms delay between steps
- **Visual States**:
  - Normal: Purple circle
  - Visiting: Orange circle (highlighted)
  - Visited: Green circle (persistent)

### Tree Operations
- **Insertion**: Path highlighting → node insertion → balancing
- **Deletion**: Node highlighting → removal → rebalancing
- **Rotation**: Animated node movement with visual feedback

---

## 📝 Code Organization

### Class Responsibilities

#### `GraphVisualization`
- Manages graph data structure
- Handles UI layout and controls
- Implements BFS/DFS algorithms
- Manages operation history
- Handles drawing and animations

#### `TheoryPage`
- Displays educational content
- Dynamic content based on data structure
- Navigation to operation page

#### `OperationPage`
- Lists available operations
- Navigation to specific visualizations

#### `MenuPage`
- Data structure selection
- Navigation to theory pages

---

## 🚀 Future Enhancements

### Potential Additions
- **Weighted Graphs**: Support for edge weights
- **Directed Graphs**: Option for directed edges
- **Shortest Path**: Dijkstra's algorithm visualization
- **Minimum Spanning Tree**: Kruskal's/Prim's algorithms
- **More Data Structures**: AVL Tree, Trie, Heap
- **Export Functionality**: Save graph as image
- **Algorithm Speed Control**: Adjustable animation speed

---

## 🐛 Known Issues & Limitations

### Current Limitations
- **Graph**: Undirected, unweighted only
- **No Self-Loops**: Graph doesn't support self-loops
- **No Parallel Edges**: Only one edge between two vertices
- **Fixed Layout**: Circular layout only (no force-directed or hierarchical)
- **History**: Not persisted between sessions

### Platform Support
- **Primary**: Windows (tested)
- **Should Work**: Linux, macOS (Qt cross-platform)

---

## 📚 Educational Value

This project serves as:
- **Learning Tool**: Visual understanding of data structures
- **Algorithm Visualization**: Step-by-step algorithm execution
- **Code Reference**: Well-structured, documented C++/Qt code
- **Project Template**: Example of Qt application architecture

---

## 👥 Development Notes

### Design Decisions
1. **QStackedWidget**: Chosen for clean page transitions
2. **Signal-Slot**: Qt's native event system for loose coupling
3. **Separate Classes**: Each visualization is independent
4. **History Logging**: Implemented for Graph and Red-Black Tree (can be extended)

### Code Style
- **Naming**: camelCase for variables, PascalCase for classes
- **Comments**: Key algorithms and complex logic documented
- **Structure**: Clear separation of UI, logic, and data

---

## 📄 License & Credits

This is an educational project developed for DSA Lab coursework.

**Technologies Used**:
- Qt Framework (https://www.qt.io/)
- CMake Build System
- C++17 Standard

---

## 🔗 Quick Reference

### Key Files
- **Navigation**: `main.cpp`
- **Graph Implementation**: `graphvisualization.h/cpp`
- **Theory Content**: `theorypage.cpp`
- **Build Config**: `CMakeLists.txt`

### Key Functions
- `GraphVisualization::addHistory()`: Logs operations
- `GraphVisualization::layoutNodes()`: Calculates node positions
- `GraphVisualization::onStartBFS()`: Initiates BFS traversal
- `GraphVisualization::onStartDFS()`: Initiates DFS traversal

---

**Last Updated**: 2024
**Version**: 1.0
**Status**: Active Development

