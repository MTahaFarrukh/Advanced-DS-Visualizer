#include "graphvisualization.h"
#include <QLinearGradient>
#include <QFont>
#include <QFontDatabase>
#include <QMessageBox>
#include <QResizeEvent>
#include <QDateTime>
#include <QtMath>

GraphVisualization::GraphVisualization(QWidget *parent)
    : QWidget(parent)
    , traversalType(TraversalType::None)
    , traversalIndex(0)
    , animTimer(new QTimer(this))
    , nextId(0)
{
    setupUI();
    setMinimumSize(900, 750);

    connect(animTimer, &QTimer::timeout, this, &GraphVisualization::onAnimationStep);
}

GraphVisualization::~GraphVisualization()
{
}

void GraphVisualization::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QVBoxLayout *topSection = new QVBoxLayout();
    topSection->setSpacing(15);

    backButton = new QPushButton("← Back to Operations", this);
    backButton->setFixedSize(160, 38);
    backButton->setCursor(Qt::PointingHandCursor);
    QFont buttonFont("Segoe UI", 11);
    backButton->setFont(buttonFont);
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(123, 79, 255, 0.1);
            color: #7b4fff;
            border: 2px solid #7b4fff;
            border-radius: 19px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: rgba(123, 79, 255, 0.2);
        }
    )");
    topSection->addWidget(backButton, 0, Qt::AlignLeft);

    titleLabel = new QLabel("Graph - Visualization", this);
    QFont titleFont("Segoe UI", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69;");
    titleLabel->setAlignment(Qt::AlignCenter);
    topSection->addWidget(titleLabel);

    instructionLabel = new QLabel("Add vertices and edges, then run BFS/DFS", this);
    QFont instructionFont("Segoe UI", 12);
    instructionLabel->setFont(instructionFont);
    instructionLabel->setStyleSheet("color: #6b5b95;");
    instructionLabel->setAlignment(Qt::AlignCenter);
    topSection->addWidget(instructionLabel);

    mainLayout->addLayout(topSection);

    // Controls - use a more organized and responsive layout
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controlsLayout->setSpacing(12);
    controlsLayout->setContentsMargins(0, 0, 0, 0);

    addVertexButton = new QPushButton("Add Vertex", this);
    addVertexButton->setMinimumSize(130, 38);
    addVertexButton->setMaximumSize(130, 38);
    addVertexButton->setCursor(Qt::PointingHandCursor);
    addVertexButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
    )");

    edgeFromCombo = new QComboBox(this);
    edgeToCombo = new QComboBox(this);
    addEdgeButton = new QPushButton("Add Edge", this);
    addEdgeButton->setMinimumSize(120, 38);
    addEdgeButton->setMaximumSize(120, 38);
    addEdgeButton->setCursor(Qt::PointingHandCursor);
    addEdgeButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
    )");

    removeVertexCombo = new QComboBox(this);
    removeVertexButton = new QPushButton("Remove Vertex", this);
    removeVertexButton->setMinimumSize(150, 38);
    removeVertexButton->setMaximumSize(150, 38);
    removeVertexButton->setCursor(Qt::PointingHandCursor);
    removeVertexButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 79, 79, 0.9);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background-color: rgba(255, 60, 60, 1);
        }
    )");

    removeEdgeFromCombo = new QComboBox(this);
    removeEdgeToCombo = new QComboBox(this);
    removeEdgeButton = new QPushButton("Remove Edge", this);
    removeEdgeButton->setMinimumSize(140, 38);
    removeEdgeButton->setMaximumSize(140, 38);
    removeEdgeButton->setCursor(Qt::PointingHandCursor);
    removeEdgeButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 79, 79, 0.9);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background-color: rgba(255, 60, 60, 1);
        }
    )");

    startCombo = new QComboBox(this);
    bfsButton = new QPushButton("Run BFS", this);
    bfsButton->setMinimumSize(120, 38);
    bfsButton->setMaximumSize(120, 38);
    bfsButton->setCursor(Qt::PointingHandCursor);
    bfsButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
    )");

    dfsButton = new QPushButton("Run DFS", this);
    dfsButton->setMinimumSize(120, 38);
    dfsButton->setMaximumSize(120, 38);
    dfsButton->setCursor(Qt::PointingHandCursor);
    dfsButton->setStyleSheet(bfsButton->styleSheet());

    clearButton = new QPushButton("Clear Graph", this);
    clearButton->setMinimumSize(140, 38);
    clearButton->setMaximumSize(140, 38);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 79, 79, 0.9);
            color: white;
            border: none;
            border-radius: 19px;
        }
        QPushButton:hover {
            background-color: rgba(255, 60, 60, 1);
        }
    )");

    // Input field for vertex ID
    vertexInput = new QLineEdit(this);
    vertexInput->setPlaceholderText("Vertex ID (optional)");
    vertexInput->setMinimumSize(120, 38);
    vertexInput->setMaximumSize(120, 38);
    vertexInput->setAlignment(Qt::AlignCenter);
    vertexInput->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 19px;
            padding: 6px 10px;
            color: #2d1b69;
        }
        QLineEdit:focus {
            border-color: #7b4fff;
        }
    )");

    // Helper to create styled labels with consistent sizing
    auto createLabel = [this](const QString &text, int minWidth = 110) -> QLabel* {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet(QString("color: #6b5b95; font-size: 11px; min-width: %1px;").arg(minWidth));
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return lbl;
    };

    // Row 1: Add Vertex and Add Edge
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(8);
    row1->setContentsMargins(0, 0, 0, 0);
    row1->addWidget(createLabel("Add Vertex:", 110));
    row1->addWidget(vertexInput);
    row1->addWidget(addVertexButton);
    row1->addSpacing(25);
    row1->addWidget(createLabel("Edge:", 50));
    row1->addWidget(edgeFromCombo);
    QLabel *arrow1 = createLabel("→", 20);
    arrow1->setAlignment(Qt::AlignCenter);
    row1->addWidget(arrow1);
    row1->addWidget(edgeToCombo);
    row1->addWidget(addEdgeButton);
    row1->addStretch();
    controlsLayout->addLayout(row1);

    // Row 2: Remove Vertex and Remove Edge
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(8);
    row2->setContentsMargins(0, 0, 0, 0);
    row2->addWidget(createLabel("Remove Vertex:", 110));
    row2->addWidget(removeVertexCombo);
    row2->addWidget(removeVertexButton);
    row2->addSpacing(25);
    row2->addWidget(createLabel("Remove Edge:", 100));
    row2->addWidget(removeEdgeFromCombo);
    QLabel *arrow2 = createLabel("→", 20);
    arrow2->setAlignment(Qt::AlignCenter);
    row2->addWidget(arrow2);
    row2->addWidget(removeEdgeToCombo);
    row2->addWidget(removeEdgeButton);
    row2->addStretch();
    controlsLayout->addLayout(row2);

    // Row 3: Traversal and Clear
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(8);
    row3->setContentsMargins(0, 0, 0, 0);
    row3->addWidget(createLabel("Traversal:", 110));
    row3->addWidget(startCombo);
    row3->addWidget(bfsButton);
    row3->addWidget(dfsButton);
    row3->addSpacing(25);
    row3->addWidget(clearButton);
    row3->addStretch();
    controlsLayout->addLayout(row3);

    // Tidy combo styling
    restyleCombos();

    // Wrap controls in a widget with centered alignment and max width for better responsiveness
    QWidget *controlsWidget = new QWidget(this);
    controlsWidget->setLayout(controlsLayout);
    controlsWidget->setMaximumWidth(1200); // Max width to prevent over-stretching
    QHBoxLayout *controlsRow = new QHBoxLayout();
    controlsRow->setContentsMargins(0, 0, 0, 0);
    controlsRow->addStretch();
    controlsRow->addWidget(controlsWidget);
    controlsRow->addStretch();
    mainLayout->addLayout(controlsRow);

    statusLabel = new QLabel("Graph is empty. Add a vertex to begin.", this);
    QFont statusFont("Segoe UI", 11);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #7b4fff; padding: 8px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Split view - Graph and History (responsive layout)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // History panel (30% width, minimum 250px)
    QVBoxLayout *historyLayout = new QVBoxLayout();
    historyLayout->setSpacing(8);
    historyLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *historyTitle = new QLabel("Operation History", this);
    historyTitle->setStyleSheet("color: #2d1b69; font-weight: bold; font-size: 14px;");
    historyLayout->addWidget(historyTitle);

    historyList = new QListWidget(this);
    historyList->setMinimumWidth(250);
    historyList->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 8px;
            padding: 5px;
            font-size: 10px;
            color: #2d1b69;
        }
        QListWidget::item {
            padding: 6px;
            border-bottom: 1px solid #f0f0f0;
            color: #2d1b69;
        }
        QListWidget::item:hover {
            background-color: #f5f0ff;
        }
        QListWidget::item:selected {
            background-color: #e8e0ff;
            color: #2d1b69;
        }
    )");
    historyLayout->addWidget(historyList);

    // Use stretch factors: 7 for graph area, 3 for history (70/30 split)
    contentLayout->addStretch(7);
    QWidget *historyWidget = new QWidget(this);
    historyWidget->setLayout(historyLayout);
    historyWidget->setMinimumWidth(250);
    contentLayout->addWidget(historyWidget, 3);

    mainLayout->addLayout(contentLayout, 1);
    setLayout(mainLayout);

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &GraphVisualization::onBackClicked);
    connect(addVertexButton, &QPushButton::clicked, this, &GraphVisualization::onAddVertexClicked);
    connect(addEdgeButton, &QPushButton::clicked, this, &GraphVisualization::onAddEdgeClicked);
    connect(removeVertexButton, &QPushButton::clicked, this, &GraphVisualization::onRemoveVertexClicked);
    connect(removeEdgeButton, &QPushButton::clicked, this, &GraphVisualization::onRemoveEdgeClicked);
    connect(clearButton, &QPushButton::clicked, this, &GraphVisualization::onClearClicked);
    connect(bfsButton, &QPushButton::clicked, this, &GraphVisualization::onStartBFS);
    connect(dfsButton, &QPushButton::clicked, this, &GraphVisualization::onStartDFS);
    connect(vertexInput, &QLineEdit::returnPressed, this, &GraphVisualization::onAddVertexClicked);

    refreshCombos();
}

void GraphVisualization::restyleCombos()
{
    QString comboStyle = R"(
        QComboBox {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 16px;
            padding: 6px 10px;
            min-width: 90px;
            max-width: 90px;
            color: #2d1b69;
        }
        QComboBox:focus {
            border-color: #7b4fff;
        }
    )";
    for (QComboBox *cb : {edgeFromCombo, edgeToCombo, removeVertexCombo, removeEdgeFromCombo, removeEdgeToCombo, startCombo}) {
        cb->setStyleSheet(comboStyle);
        cb->setMinimumSize(90, 38);
        cb->setMaximumSize(90, 38);
    }
}

void GraphVisualization::refreshCombos()
{
    QStringList ids;
    ids.reserve(nodes.size());
    for (const auto &n : nodes) ids << QString::number(n.id);

    auto fill = [&](QComboBox *cb) {
        cb->blockSignals(true);
        cb->clear();
        cb->addItems(ids);
        cb->blockSignals(false);
    };

    fill(edgeFromCombo);
    fill(edgeToCombo);
    fill(removeVertexCombo);
    fill(removeEdgeFromCombo);
    fill(removeEdgeToCombo);
    fill(startCombo);
}

void GraphVisualization::setControlsEnabled(bool enabled)
{
    addVertexButton->setEnabled(enabled);
    addEdgeButton->setEnabled(enabled);
    removeVertexButton->setEnabled(enabled);
    removeEdgeButton->setEnabled(enabled);
    clearButton->setEnabled(enabled);
    bfsButton->setEnabled(enabled);
    dfsButton->setEnabled(enabled);
    edgeFromCombo->setEnabled(enabled);
    edgeToCombo->setEnabled(enabled);
    removeVertexCombo->setEnabled(enabled);
    removeEdgeFromCombo->setEnabled(enabled);
    removeEdgeToCombo->setEnabled(enabled);
    startCombo->setEnabled(enabled);
    vertexInput->setEnabled(enabled);
}

void GraphVisualization::onBackClicked()
{
    if (traversalType != TraversalType::None) {
        animTimer->stop();
        traversalType = TraversalType::None;
    }
    emit backToOperations();
}

void GraphVisualization::onAddVertexClicked()
{
    int id = -1;
    QString text = vertexInput->text().trimmed();
    if (!text.isEmpty()) {
        bool ok;
        id = text.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer for vertex ID.");
            return;
        }
        // Check if ID already exists
        for (const auto &n : nodes) {
            if (n.id == id) {
                QMessageBox::warning(this, "Duplicate ID", QString("Vertex %1 already exists.").arg(id));
                return;
            }
        }
    } else {
        // Auto-generate ID
        id = nextId++;
    }
    
    nodes.append(GraphNode(id, QPointF(0, 0)));
    adjacency.insert(id, QSet<int>());
    if (id >= nextId) nextId = id + 1;
    
    layoutNodes();
    refreshCombos();
    vertexInput->clear();
    statusLabel->setText(QString("Vertex %1 added.").arg(id));
    addHistory("ADD_VERTEX", id, -1, QString("Vertex %1 added to graph").arg(id));
    update();
}

void GraphVisualization::onAddEdgeClicked()
{
    bool ok1 = false, ok2 = false;
    int u = edgeFromCombo->currentText().toInt(&ok1);
    int v = edgeToCombo->currentText().toInt(&ok2);
    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "Invalid", "Please select valid vertices.");
        return;
    }
    if (u == v) {
        QMessageBox::warning(this, "Invalid", "Self-loops are not allowed.");
        return;
    }
    if (addEdgeInternal(u, v)) {
        statusLabel->setText(QString("Edge %1-%2 added.").arg(u).arg(v));
        addHistory("ADD_EDGE", u, v, QString("Edge %1-%2 added").arg(u).arg(v));
        update();
    } else {
        statusLabel->setText("Edge already exists or vertices missing.");
        addHistory("ADD_EDGE", u, v, QString("Failed: Edge %1-%2 already exists").arg(u).arg(v));
    }
}

void GraphVisualization::onRemoveVertexClicked()
{
    bool ok = false;
    int u = removeVertexCombo->currentText().toInt(&ok);
    if (!ok) return;
    if (removeVertexInternal(u)) {
        layoutNodes();
        refreshCombos();
        statusLabel->setText(QString("Vertex %1 removed.").arg(u));
        addHistory("REMOVE_VERTEX", u, -1, QString("Vertex %1 and all its edges removed").arg(u));
        update();
    } else {
        addHistory("REMOVE_VERTEX", u, -1, QString("Failed: Vertex %1 not found").arg(u));
    }
}

void GraphVisualization::onRemoveEdgeClicked()
{
    bool ok1 = false, ok2 = false;
    int u = removeEdgeFromCombo->currentText().toInt(&ok1);
    int v = removeEdgeToCombo->currentText().toInt(&ok2);
    if (!ok1 || !ok2) return;
    if (removeEdgeInternal(u, v)) {
        statusLabel->setText(QString("Edge %1-%2 removed.").arg(u).arg(v));
        addHistory("REMOVE_EDGE", u, v, QString("Edge %1-%2 removed").arg(u).arg(v));
        update();
    } else {
        addHistory("REMOVE_EDGE", u, v, QString("Failed: Edge %1-%2 not found").arg(u).arg(v));
    }
}

void GraphVisualization::onClearClicked()
{
    nodes.clear();
    adjacency.clear();
    nextId = 0;
    traversalType = TraversalType::None;
    frontier.clear();
    traversalOrder.clear();
    traversalIndex = 0;
    animTimer->stop();
    history.clear();
    historyList->clear();
    refreshCombos();
    statusLabel->setText("Graph cleared! Add a vertex to begin.");
    addHistory("CLEAR", -1, -1, "Entire graph cleared");
    update();
}

void GraphVisualization::onStartBFS()
{
    bool ok = false;
    int s = startCombo->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid", "Select a start vertex.");
        return;
    }
    if (!adjacency.contains(s)) {
        QMessageBox::warning(this, "Invalid", "Start vertex not found.");
        return;
    }
    resetHighlights();
    traversalType = TraversalType::BFS;
    traversalOrder.clear();
    frontier.clear();
    traversalIndex = 0;

    // BFS precompute order
    QSet<int> visited;
    QList<int> queue;
    queue.append(s);
    visited.insert(s);
    while (!queue.isEmpty()) {
        int u = queue.front();
        queue.pop_front();
        traversalOrder.append(u);
        for (int v : adjacency.value(u)) {
            if (!visited.contains(v)) {
                visited.insert(v);
                queue.append(v);
            }
        }
    }

    setControlsEnabled(false);
    animTimer->start(700);
    statusLabel->setText("Running BFS...");
    addHistory("BFS", s, -1, QString("BFS traversal started from vertex %1").arg(s));
}

void GraphVisualization::onStartDFS()
{
    bool ok = false;
    int s = startCombo->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid", "Select a start vertex.");
        return;
    }
    if (!adjacency.contains(s)) {
        QMessageBox::warning(this, "Invalid", "Start vertex not found.");
        return;
    }
    resetHighlights();
    traversalType = TraversalType::DFS;
    traversalOrder.clear();
    frontier.clear();
    traversalIndex = 0;

    // DFS precompute order
    QSet<int> visited;
    QList<int> stack;
    stack.append(s);
    while (!stack.isEmpty()) {
        int u = stack.takeLast();
        if (visited.contains(u)) continue;
        visited.insert(u);
        traversalOrder.append(u);
        // push neighbors in reverse for a stable ordering
        QList<int> neigh = adjacency.value(u).values();
        std::sort(neigh.begin(), neigh.end(), std::greater<int>());
        for (int v : neigh) {
            if (!visited.contains(v)) stack.append(v);
        }
    }

    setControlsEnabled(false);
    animTimer->start(700);
    statusLabel->setText("Running DFS...");
    addHistory("DFS", s, -1, QString("DFS traversal started from vertex %1").arg(s));
}

void GraphVisualization::onAnimationStep()
{
    if (traversalIndex > 0 && traversalIndex <= traversalOrder.size()) {
        int prev = traversalOrder[traversalIndex - 1];
        // Mark previous as visited (persist green), remove highlight
        for (auto &n : nodes) if (n.id == prev) { n.visited = true; n.highlighted = false; }
    }

    if (traversalIndex >= traversalOrder.size()) {
        QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
        animTimer->stop();
        traversalType = TraversalType::None;
        setControlsEnabled(true);
        statusLabel->setText("Traversal complete.");
        addHistory(algo, -1, -1, QString("%1 traversal completed. Visited %2 vertices").arg(algo).arg(traversalOrder.size()));
        update();
        return;
    }

    int u = traversalOrder[traversalIndex];
    for (auto &n : nodes) if (n.id == u) { n.highlighted = true; }
    statusLabel->setText(QString("Visiting %1").arg(u));
    traversalIndex++;
    update();
}

void GraphVisualization::layoutNodes()
{
    if (nodes.isEmpty()) return;
    
    // Calculate canvas area dynamically based on current widget size
    // Canvas takes 70% width (history panel takes 30%)
    int topMargin = 320;
    int bottomMargin = 30;
    int sideMargin = 30;
    int canvasWidth = (int)(width() * 0.65);
    QRect canvasRect(sideMargin, topMargin, canvasWidth, height() - topMargin - bottomMargin);
    
    if (canvasRect.width() <= 0 || canvasRect.height() <= 0) return;
    
    QPointF center(canvasRect.center());
    double radius = qMin(canvasRect.width(), canvasRect.height()) * 0.35;
    if (radius < 50) radius = 50; // Minimum radius
    
    int n = nodes.size();
    for (int i = 0; i < n; ++i) {
        double angle = (2 * M_PI * i) / n;
        nodes[i].pos = QPointF(
            center.x() + radius * qCos(angle),
            center.y() + radius * qSin(angle)
        );
    }
}

void GraphVisualization::resetHighlights()
{
    for (auto &n : nodes) { n.highlighted = false; n.visited = false; }
}


bool GraphVisualization::addEdgeInternal(int u, int v)
{
    if (!adjacency.contains(u) || !adjacency.contains(v)) return false;
    if (adjacency[u].contains(v)) return false;
    adjacency[u].insert(v);
    adjacency[v].insert(u);
    return true;
}

bool GraphVisualization::removeVertexInternal(int u)
{
    if (!adjacency.contains(u)) return false;
    // remove edges
    for (int v : adjacency[u]) adjacency[v].remove(u);
    adjacency.remove(u);
    // remove node
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].id == u) { nodes.removeAt(i); break; }
    }
    return true;
}

bool GraphVisualization::removeEdgeInternal(int u, int v)
{
    if (!adjacency.contains(u) || !adjacency.contains(v)) return false;
    bool existed = adjacency[u].remove(v) | adjacency[v].remove(u);
    return existed;
}

void GraphVisualization::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));
    gradient.setColorAt(1.0, QColor(237, 228, 255));
    painter.fillRect(rect(), gradient);

    // Canvas area - recalculate layout before drawing
    layoutNodes();
    
    int topMargin = 320;
    int bottomMargin = 30;
    int sideMargin = 30;
    int canvasY = topMargin;
    int canvasHeight = height() - canvasY - bottomMargin;
    // Canvas takes 70% width (history panel takes 30%)
    int canvasWidth = (int)(width() * 0.65);
    QRect canvasRect(sideMargin, canvasY, canvasWidth, canvasHeight);
    
    if (canvasRect.width() > 0 && canvasRect.height() > 0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(canvasRect, 16, 16);
    }

    drawGraph(painter);
}

void GraphVisualization::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutNodes();
    update();
}

void GraphVisualization::drawGraph(QPainter &painter)
{
    if (nodes.isEmpty()) return;

    // Draw edges first
    painter.setPen(QPen(QColor(123, 79, 255), 2));
    for (auto it = adjacency.constBegin(); it != adjacency.constEnd(); ++it) {
        int u = it.key();
        for (int v : it.value()) {
            if (u < v) {
                const GraphNode *nu = nullptr, *nv = nullptr;
                for (const auto &n : nodes) {
                    if (n.id == u) nu = &n;
                    if (n.id == v) nv = &n;
                }
                if (nu && nv) drawEdge(painter, nu->pos, nv->pos, false);
            }
        }
    }

    // Draw nodes on top
    for (const auto &n : nodes) drawNode(painter, n);
}

void GraphVisualization::drawEdge(QPainter &painter, const QPointF &a, const QPointF &b, bool highlighted) const
{
    QPen pen(highlighted ? QColor(255, 165, 0) : QColor(123, 79, 255));
    pen.setWidth(highlighted ? 4 : 2);
    painter.setPen(pen);
    painter.drawLine(a, b);
}

void GraphVisualization::drawNode(QPainter &painter, const GraphNode &node) const
{
    if (node.highlighted) {
        painter.setPen(QPen(QColor(255, 165, 0), 4));
        painter.setBrush(QColor(255, 200, 100));
    } else if (node.visited) {
        painter.setPen(QPen(QColor(50, 205, 50), 4));
        painter.setBrush(QColor(144, 238, 144));
    } else {
        painter.setPen(QPen(QColor(123, 79, 255), 3));
        painter.setBrush(QColor(200, 180, 255));
    }

    painter.drawEllipse(node.pos, NODE_RADIUS, NODE_RADIUS);

    painter.setPen(Qt::black);
    QFont font("Segoe UI", 12, QFont::Bold);
    painter.setFont(font);
    QRectF r(node.pos.x() - NODE_RADIUS, node.pos.y() - NODE_RADIUS, NODE_RADIUS * 2, NODE_RADIUS * 2);
    painter.drawText(r, Qt::AlignCenter, QString::number(node.id));
}

void GraphVisualization::addHistory(const QString &operation, int value1, int value2, const QString &description)
{
    GraphHistoryEntry entry;
    entry.operation = operation;
    entry.value1 = value1;
    entry.value2 = value2;
    entry.description = description;
    entry.timestamp = getCurrentTime();

    history.append(entry);

    QString displayText;
    if (value2 == -1) {
        if (value1 == -1) {
            displayText = QString("[%1] %2: %3")
                              .arg(entry.timestamp)
                              .arg(entry.operation)
                              .arg(entry.description);
        } else {
            displayText = QString("[%1] %2 (%3): %4")
                              .arg(entry.timestamp)
                              .arg(entry.operation)
                              .arg(value1)
                              .arg(entry.description);
        }
    } else {
        displayText = QString("[%1] %2 (%3-%4): %5")
                          .arg(entry.timestamp)
                          .arg(entry.operation)
                          .arg(value1)
                          .arg(value2)
                          .arg(entry.description);
    }

    QListWidgetItem *item = new QListWidgetItem(displayText);
    item->setForeground(QColor("#2d1b69"));
    historyList->addItem(item);
    historyList->scrollToBottom();
}

QString GraphVisualization::getCurrentTime()
{
    return QDateTime::currentDateTime().toString("HH:mm:ss");
}




