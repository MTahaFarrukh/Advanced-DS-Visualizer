#include "hashmapvisualization.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QSplitterHandle>

// Define static constants
const int HashMapVisualization::BUCKET_WIDTH = 80;
const int HashMapVisualization::BUCKET_HEIGHT = 60;
const int HashMapVisualization::BUCKET_SPACING = 10;
const int HashMapVisualization::CHAIN_ITEM_HEIGHT = 25;
const int HashMapVisualization::MAX_VISIBLE_BUCKETS = 12;

HashMapVisualization::HashMapVisualization(QWidget *parent)
    : QWidget(parent)
    , hashMap(new HashMap(10, 10.0f))  // 10 buckets, high load factor to prevent rehashing
    , animationTimer(new QTimer(this))
    , highlightRect(nullptr)
{
    setupUI();
    updateVisualization();
    updateStepTrace();
    
    // Connect timer for delayed updates
    animationTimer->setSingleShot(true);
    connect(animationTimer, &QTimer::timeout, this, &HashMapVisualization::updateVisualization);
    
    setMinimumSize(1200, 800);
}

HashMapVisualization::~HashMapVisualization()
{
    delete hashMap;
}

void HashMapVisualization::setupUI()
{
    // Main splitter for left (visualization) and right (controls + trace) panels
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background-color: #e0e0e0;
            width: 2px;
        }
        QSplitter::handle:hover {
            background-color: #7b4fff;
        }
    )");
    
    setupVisualizationArea();
    setupRightPanel();
    
    // Set splitter proportions (65% visualization, 35% controls+trace)
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({780, 420});
    
    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainSplitter);
    setLayout(mainLayout);
}

void HashMapVisualization::setupVisualizationArea()
{
    leftPanel = new QWidget();
    leftPanel->setStyleSheet("background: transparent;");
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(40, 30, 20, 30);
    leftLayout->setSpacing(25);
    
    // Header with back button and title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    backButton = new QPushButton("← Back to Operations");
    backButton->setFixedSize(180, 45);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(74, 144, 226, 0.1);
            color: #4a90e2;
            border: 2px solid rgba(74, 144, 226, 0.3);
            border-radius: 22px;
            padding: 10px 20px;
            font-family: 'Segoe UI';
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(74, 144, 226, 0.2);
            border-color: #4a90e2;
        }
        QPushButton:pressed {
            background-color: rgba(74, 144, 226, 0.3);
        }
    )");
    connect(backButton, &QPushButton::clicked, this, &HashMapVisualization::onBackClicked);
    
    titleLabel = new QLabel("Generic Hash Table");
    QFont titleFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "SF Pro Display", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();
    
    leftLayout->addLayout(headerLayout);
    
    // Stats at top-left of visualization area
    setupStatsTopLeft();
    
    // Visualization area with gradient background (fixed size, no scroll)
    scene = new QGraphicsScene(this);
    
    visualizationView = new QGraphicsView(scene);
    visualizationView->setRenderHint(QPainter::Antialiasing);
    visualizationView->setFixedHeight(450);
    visualizationView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    visualizationView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    visualizationView->setStyleSheet(R"(
        QGraphicsView {
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 20px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(248, 251, 255, 0.98),
                stop:1 rgba(240, 247, 255, 0.98));
        }
    )");
    
    // Add drop shadow to visualization area
    QGraphicsDropShadowEffect *viewShadow = new QGraphicsDropShadowEffect();
    viewShadow->setBlurRadius(20);
    viewShadow->setXOffset(0);
    viewShadow->setYOffset(6);
    viewShadow->setColor(QColor(74, 144, 226, 25));
    visualizationView->setGraphicsEffect(viewShadow);
    
    leftLayout->addWidget(visualizationView, 1);
    
    // Bottom note about bucket limitation
    QLabel *bucketNote = new QLabel("* Limited to 10 buckets for screen visibility");
    bucketNote->setStyleSheet(R"(
        QLabel {
            color: #7f8c8d;
            font-size: 11px;
            font-style: italic;
            padding: 5px;
        }
    )");
    bucketNote->setAlignment(Qt::AlignLeft);
    leftLayout->addWidget(bucketNote);
}

void HashMapVisualization::setupStatsTopLeft()
{
    // Stats at top-left of visualization area
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    sizeLabel = new QLabel("Size: 0");
    bucketCountLabel = new QLabel("Buckets: 10");
    loadFactorLabel = new QLabel("Load Factor: 0.00");
    
    QString statsStyle = R"(
        QLabel {
            color: #34495e;
            font-weight: bold;
            font-family: 'Segoe UI';
            font-size: 12px;
            padding: 8px 12px;
            background-color: rgba(74, 144, 226, 0.1);
            border-radius: 12px;
            border: 1px solid rgba(74, 144, 226, 0.2);
        }
    )";
    
    sizeLabel->setStyleSheet(statsStyle);
    bucketCountLabel->setStyleSheet(statsStyle);
    loadFactorLabel->setStyleSheet(statsStyle);
    
    statsLayout->addWidget(sizeLabel);
    statsLayout->addWidget(bucketCountLabel);
    statsLayout->addWidget(loadFactorLabel);
    statsLayout->addStretch();
    
    leftLayout->addLayout(statsLayout);
}

void HashMapVisualization::setupRightPanel()
{
    rightPanel = new QWidget();
    rightPanel->setMinimumWidth(400);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(248, 251, 255, 0.98),
                stop:1 rgba(240, 247, 255, 0.98));
        }
    )");
    
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(15);
    
    // Split right panel: chat history top, controls bottom
    setupStepTraceTop();
    setupTypeSelection();
    setupControls();
}

void HashMapVisualization::setupStepTraceTop()
{
    // Chat history at top of right panel
    traceGroup = new QGroupBox("Operation History");
    traceGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2c3e50;
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 10px;
            background: rgba(255, 255, 255, 0.7);
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px 0 8px;
            background: rgba(255, 255, 255, 0.9);
            border-radius: 6px;
        }
    )");
    
    QVBoxLayout *traceLayout = new QVBoxLayout(traceGroup);
    traceLayout->setContentsMargins(15, 20, 15, 15);
    
    stepsList = new QListWidget();
    stepsList->setMinimumHeight(250);
    stepsList->setStyleSheet(R"(
        QListWidget {
            background: rgba(255, 255, 255, 0.9);
            border: 1px solid rgba(74, 144, 226, 0.15);
            border-radius: 8px;
            padding: 8px;
            font-family: 'Segoe UI';
            font-size: 12px;
            selection-background-color: rgba(74, 144, 226, 0.2);
        }
        QListWidget::item {
            padding: 8px 12px;
            margin: 2px 0px;
            border-radius: 6px;
            background-color: rgba(248, 251, 255, 0.8);
            border: 1px solid rgba(74, 144, 226, 0.1);
            color: #2c3e50;
        }
        QListWidget::item:hover {
            background-color: rgba(74, 144, 226, 0.1);
            border-color: rgba(74, 144, 226, 0.2);
        }
        QListWidget::item:selected {
            background: rgba(74, 144, 226, 0.15);
            color: #2c3e50;
            border-color: #4a90e2;
        }
        QScrollBar:vertical {
            background-color: rgba(74, 144, 226, 0.05);
            width: 12px;
            border-radius: 6px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background-color: rgba(74, 144, 226, 0.3);
            border-radius: 5px;
            min-height: 20px;
            margin: 1px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgba(74, 144, 226, 0.5);
        }
        QScrollBar::handle:vertical:pressed {
            background-color: #4a90e2;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
    
    traceLayout->addWidget(stepsList);
    rightLayout->addWidget(traceGroup, 2);  // Give it more space (2/3 of right panel)
}

void HashMapVisualization::setupTypeSelection()
{
    typeGroup = new QGroupBox("Data Types");
    typeGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2c3e50;
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 10px;
            background: rgba(255, 255, 255, 0.8);
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px 0 8px;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 6px;
            color: #2c3e50;
        }
    )");
    
    QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
    
    QLabel *keyLabel = new QLabel("Key:");
    keyLabel->setStyleSheet("color: #2c3e50; font-weight: bold; font-size: 12px;");
    keyTypeCombo = new QComboBox();
    keyTypeCombo->addItems({"String", "Integer", "Double", "Float"});
    
    QLabel *valueLabel = new QLabel("Value:");
    valueLabel->setStyleSheet("color: #2c3e50; font-weight: bold; font-size: 12px;");
    valueTypeCombo = new QComboBox();
    valueTypeCombo->addItems({"String", "Integer", "Double", "Float"});
    
    QString comboStyle = R"(
        QComboBox {
            border: 2px solid rgba(74, 144, 226, 0.3);
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 12px;
            font-weight: 500;
            background: white;
            color: #2c3e50;
            min-width: 90px;
            selection-background-color: rgba(74, 144, 226, 0.2);
        }
        QComboBox:hover {
            border-color: rgba(74, 144, 226, 0.6);
            background: rgba(248, 251, 255, 1.0);
        }
        QComboBox:focus {
            border-color: #4a90e2;
            background: white;
        }
        QComboBox:on {
            border-color: #4a90e2;
            background: rgba(248, 251, 255, 1.0);
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid rgba(74, 144, 226, 0.3);
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
            background: rgba(74, 144, 226, 0.05);
        }
        QComboBox::drop-down:hover {
            background: rgba(74, 144, 226, 0.1);
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #4a90e2;
            margin: 0px;
        }
        QComboBox::down-arrow:hover {
            border-top-color: #2980b9;
        }
        QComboBox QAbstractItemView {
            border: 2px solid rgba(74, 144, 226, 0.3);
            border-radius: 8px;
            background: white;
            color: #2c3e50;
            selection-background-color: rgba(74, 144, 226, 0.15);
            selection-color: #2c3e50;
            outline: none;
        }
        QComboBox QAbstractItemView::item {
            padding: 8px 12px;
            border: none;
            min-height: 20px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: rgba(74, 144, 226, 0.1);
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: rgba(74, 144, 226, 0.2);
            color: #2c3e50;
        }
    )";
    
    keyTypeCombo->setStyleSheet(comboStyle);
    valueTypeCombo->setStyleSheet(comboStyle);
    
    // Horizontal layout: Key [dropdown] Value [dropdown]
    typeLayout->addWidget(keyLabel);
    typeLayout->addWidget(keyTypeCombo);
    typeLayout->addWidget(valueLabel);
    typeLayout->addWidget(valueTypeCombo);
    typeLayout->addStretch();
    
    connect(keyTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HashMapVisualization::onTypeChanged);
    connect(valueTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HashMapVisualization::onTypeChanged);
    
    rightLayout->addWidget(typeGroup);
}

void HashMapVisualization::setupControls()
{
    controlGroup = new QGroupBox("Operations");
    controlGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2c3e50;
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 10px;
            background: rgba(255, 255, 255, 0.8);
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px 0 8px;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 6px;
            color: #2c3e50;
        }
    )");
    
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    
    // Input fields
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    keyInput = new QLineEdit();
    keyInput->setPlaceholderText("Enter key (leave empty to search by value)");
    valueInput = new QLineEdit();
    valueInput->setPlaceholderText("Enter value (leave empty to search by key)");
    
    QString inputStyle = R"(
        QLineEdit {
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 12px;
            background: white;
            color: #2c3e50;
        }
        QLineEdit:focus {
            border-color: #4a90e2;
        }
        QLineEdit::placeholder {
            color: #7f8c8d;
        }
    )";
    
    keyInput->setStyleSheet(inputStyle);
    valueInput->setStyleSheet(inputStyle);
    
    inputLayout->addWidget(keyInput);
    inputLayout->addWidget(valueInput);
    
    controlLayout->addLayout(inputLayout);
    
    // Buttons
    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    
    insertButton = new QPushButton("Insert");
    searchButton = new QPushButton("Search");
    deleteButton = new QPushButton("Delete");
    clearButton = new QPushButton("Clear");
    randomizeButton = new QPushButton("Random");
    
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px;
            font-weight: bold;
            min-height: 30px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
    )";
    
    insertButton->setStyleSheet(buttonStyle);
    searchButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(buttonStyle);
    clearButton->setStyleSheet(buttonStyle);
    randomizeButton->setStyleSheet(buttonStyle);
    
    buttonLayout1->addWidget(insertButton);
    buttonLayout1->addWidget(searchButton);
    buttonLayout1->addWidget(deleteButton);
    
    buttonLayout2->addWidget(clearButton);
    buttonLayout2->addWidget(randomizeButton);
    
    controlLayout->addLayout(buttonLayout1);
    controlLayout->addLayout(buttonLayout2);
    
    // Connect signals
    connect(insertButton, &QPushButton::clicked, this, &HashMapVisualization::onInsertClicked);
    connect(searchButton, &QPushButton::clicked, this, &HashMapVisualization::onSearchClicked);
    connect(deleteButton, &QPushButton::clicked, this, &HashMapVisualization::onDeleteClicked);
    connect(clearButton, &QPushButton::clicked, this, &HashMapVisualization::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &HashMapVisualization::onRandomizeClicked);
    
    rightLayout->addWidget(controlGroup);
}

void HashMapVisualization::setupStats()
{
    statsGroup = new QGroupBox("Statistics");
    statsGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2d1b69;
            border: 2px solid rgba(123, 79, 255, 0.2);
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
    QHBoxLayout *statsLayout = new QHBoxLayout(statsGroup);
    
    sizeLabel = new QLabel("Size: 0");
    bucketCountLabel = new QLabel("Buckets: 10");
    loadFactorLabel = new QLabel("Load: 0.00");
    
    QString statsStyle = R"(
        QLabel {
            background: rgba(123, 79, 255, 0.1);
            border-radius: 6px;
            padding: 5px 8px;
            font-weight: bold;
        }
    )";
    
    sizeLabel->setStyleSheet(statsStyle);
    bucketCountLabel->setStyleSheet(statsStyle);
    loadFactorLabel->setStyleSheet(statsStyle);
    
    statsLayout->addWidget(sizeLabel);
    statsLayout->addWidget(bucketCountLabel);
    statsLayout->addWidget(loadFactorLabel);
    
    rightLayout->addWidget(statsGroup);
}

void HashMapVisualization::setupStepTrace()
{
    traceGroup = new QGroupBox("Operation History");
    traceGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2d1b69;
            border: 2px solid rgba(123, 79, 255, 0.2);
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
    QVBoxLayout *traceLayout = new QVBoxLayout(traceGroup);
    
    stepsList = new QListWidget();
    stepsList->setStyleSheet(R"(
        QListWidget {
            background: white;
            border: 1px solid rgba(123, 79, 255, 0.2);
            border-radius: 8px;
            padding: 5px;
            font-family: 'Segoe UI';
            font-size: 12px;
        }
        QListWidget::item {
            padding: 6px;
            border-bottom: 1px solid #f0f0f0;
        }
        QListWidget::item:selected {
            background: rgba(123, 79, 255, 0.1);
        }
    )");
    
    traceLayout->addWidget(stepsList);
    rightLayout->addWidget(traceGroup, 1);  // Give it more space
}


void HashMapVisualization::drawBuckets()
{
    // Clear existing visualization
    scene->clear();
    bucketRects.clear();
    bucketTexts.clear();
    chainTexts.clear();
    
    const int bucketCount = 10; // Fixed 10 buckets
    const QVector<int> bucketSizes = hashMap->bucketSizes();
    const QVector<QVector<QPair<QVariant, QVariant>>> bucketContents = hashMap->getBucketContents();
    
    // Calculate layout for 10 buckets in a single row
    const int totalWidth = bucketCount * (BUCKET_WIDTH + BUCKET_SPACING) - BUCKET_SPACING;
    const int startX = -totalWidth / 2;
    
    bucketRects.resize(bucketCount);
    bucketTexts.resize(bucketCount);
    chainTexts.resize(bucketCount);
    
    for (int i = 0; i < bucketCount; ++i) {
        const int x = startX + i * (BUCKET_WIDTH + BUCKET_SPACING);
        const int y = 0;
        
        // Calculate dynamic bucket height based on content
        int bucketHeight = BUCKET_HEIGHT;
        if (i < bucketContents.size() && !bucketContents[i].isEmpty()) {
            bucketHeight = BUCKET_HEIGHT + (bucketContents[i].size() * 30); // 30px per item
        }
        
        // Create bucket with dynamic height
        QGraphicsPathItem *bucketPath = new QGraphicsPathItem();
        QPainterPath path;
        path.addRoundedRect(QRectF(x, y, BUCKET_WIDTH, bucketHeight), 12, 12);
        bucketPath->setPath(path);
        
        // Set gradient brush for bucket
        QLinearGradient bucketGradient(x, y, x, y + bucketHeight);
        if (bucketSizes[i] > 0) {
            // Filled bucket - purple gradient
            bucketGradient.setColorAt(0.0, QColor(123, 79, 255, 15));
            bucketGradient.setColorAt(1.0, QColor(123, 79, 255, 25));
        } else {
            // Empty bucket - light gradient
            bucketGradient.setColorAt(0.0, QColor(255, 255, 255, 200));
            bucketGradient.setColorAt(1.0, QColor(250, 248, 255, 200));
        }
        bucketPath->setBrush(QBrush(bucketGradient));
        bucketPath->setPen(QPen(QColor(123, 79, 255, 120), 2.5));
        scene->addItem(bucketPath);
        
        // Bucket index label
        QGraphicsTextItem *indexText = scene->addText(QString::number(i));
        indexText->setPos(x + BUCKET_WIDTH/2 - 8, y - 35);
        indexText->setDefaultTextColor(QColor(45, 27, 105));
        QFont indexFont("Segoe UI", 14);
        indexFont.setBold(true);
        indexText->setFont(indexFont);
        
        // Show data directly inside the bucket
        QVector<QGraphicsTextItem*> chainItems;
        if (i < bucketContents.size()) {
            const auto &items = bucketContents[i];
            for (int j = 0; j < items.size(); ++j) {
                const int itemY = y + 10 + j * 30; // Items stacked vertically inside bucket
                
                // Chain item background inside bucket
                QGraphicsPathItem *itemBgPath = new QGraphicsPathItem();
                QPainterPath itemPath;
                itemPath.addRoundedRect(QRectF(x + 4, itemY, BUCKET_WIDTH - 8, 25), 6, 6);
                itemBgPath->setPath(itemPath);
                itemBgPath->setBrush(QBrush(QColor(255, 255, 255, 180)));
                itemBgPath->setPen(QPen(QColor(123, 79, 255, 100), 1.5));
                itemBgPath->setZValue(1);
                scene->addItem(itemBgPath);
                
                // Chain item text with actual key-value pair
                const QVariant &key = items[j].first;
                const QVariant &value = items[j].second;
                QString keyStr = HashMap::variantToDisplayString(key);
                QString valueStr = HashMap::variantToDisplayString(value);
                QString displayText = QString("%1→%2").arg(keyStr.left(4), valueStr.left(4));
                
                QGraphicsTextItem *chainItem = scene->addText(displayText);
                chainItem->setPos(x + 6, itemY + 2);
                chainItem->setDefaultTextColor(QColor(45, 27, 105));
                QFont chainFont("Segoe UI", 8);
                chainFont.setBold(true);
                chainItem->setFont(chainFont);
                chainItem->setZValue(2);
                chainItems.append(chainItem);
                
                // Add chain link arrow for multiple items
                if (j > 0) {
                    QGraphicsTextItem *arrow = scene->addText("↓");
                    arrow->setPos(x + BUCKET_WIDTH/2 - 5, itemY - 15);
                    arrow->setDefaultTextColor(QColor(123, 79, 255, 150));
                    QFont arrowFont("Segoe UI", 10);
                    arrowFont.setBold(true);
                    arrow->setFont(arrowFont);
                    arrow->setZValue(2);
                }
            }
        }
        chainTexts[i] = chainItems;
        
        // Empty bucket label
        if (bucketSizes[i] == 0) {
            QGraphicsTextItem *emptyText = scene->addText("empty");
            emptyText->setPos(x + BUCKET_WIDTH/2 - 15, y + BUCKET_HEIGHT/2 - 10);
            emptyText->setDefaultTextColor(QColor(150, 150, 150));
            QFont emptyFont("Segoe UI", 9);
            emptyFont.setItalic(true);
            emptyText->setFont(emptyFont);
            emptyText->setZValue(2);
        }
    }
    
    // Add title higher up
    QGraphicsTextItem *vizTitle = scene->addText("Hash Table (Open Chaining)");
    QFont titleFont("Segoe UI", 16);
    titleFont.setBold(true);
    vizTitle->setFont(titleFont);
    vizTitle->setDefaultTextColor(QColor(44, 62, 80));
    vizTitle->setPos(-totalWidth/2, -120);
    
    
    // Adjust scene rect with padding
    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-60, -100, 60, 80));
}

void HashMapVisualization::updateVisualization()
{
    drawBuckets();
    showStats();
}

void HashMapVisualization::updateStepTrace()
{
    stepsList->clear();
    const QVector<QString> &steps = hashMap->lastSteps();
    
    for (int i = 0; i < steps.size(); ++i) {
        const QString &step = steps[i];
        
        // Add step with better formatting
        QListWidgetItem *item = new QListWidgetItem(step);
        
        // Add spacing between different operations
        if (step.startsWith("🔍") || step.startsWith("➕") || step.startsWith("❌") || step.startsWith("🗑️")) {
            // This is a new operation - add some visual separation
            if (i > 0) {
                QListWidgetItem *separator = new QListWidgetItem("────────────────────");
                separator->setTextAlignment(Qt::AlignCenter);
                separator->setFlags(Qt::NoItemFlags); // Make it non-selectable
                separator->setForeground(QColor(189, 195, 199));
                stepsList->addItem(separator);
            }
        }
        
        // Style different types of steps
        if (step.contains("✅")) {
            item->setForeground(QColor(39, 174, 96)); // Green for success
        } else if (step.contains("❌")) {
            item->setForeground(QColor(231, 76, 60)); // Red for failure
        } else if (step.contains("🔍")) {
            item->setForeground(QColor(52, 152, 219)); // Blue for search
        } else if (step.contains("📊") || step.contains("🎯")) {
            item->setForeground(QColor(155, 89, 182)); // Purple for calculation
        } else {
            item->setForeground(QColor(44, 62, 80)); // Default dark color
        }
        
        stepsList->addItem(item);
    }
    
    // Auto-scroll to bottom to show latest steps
    if (stepsList->count() > 0) {
        stepsList->scrollToBottom();
    }
}

void HashMapVisualization::showStats()
{
    sizeLabel->setText(QString("Size: %1").arg(hashMap->size()));
    bucketCountLabel->setText(QString("Buckets: 10"));  // Fixed to 10
    loadFactorLabel->setText(QString("Load Factor: %1").arg(hashMap->loadFactor(), 0, 'f', 2));
}

void HashMapVisualization::animateOperation(const QString &operation)
{
    Q_UNUSED(operation);
    // TODO: Add specific animations for different operations
    updateVisualization();
    updateStepTrace();
}

void HashMapVisualization::onBackClicked()
{
    emit backToOperations();
}

void HashMapVisualization::onTypeChanged()
{
    // Update HashMap data types based on combo box selections
    HashMap::DataType keyType = static_cast<HashMap::DataType>(keyTypeCombo->currentIndex());
    HashMap::DataType valueType = static_cast<HashMap::DataType>(valueTypeCombo->currentIndex());
    
    hashMap->setKeyType(keyType);
    hashMap->setValueType(valueType);
    
    // Update input placeholders based on selected types
    QString keyPlaceholder = QString("Enter %1 key").arg(HashMap::dataTypeToString(keyType).toLower());
    QString valuePlaceholder = QString("Enter %1 value").arg(HashMap::dataTypeToString(valueType).toLower());
    
    keyInput->setPlaceholderText(keyPlaceholder);
    valueInput->setPlaceholderText(valuePlaceholder);
    
    // Clear existing data when types change
    hashMap->clear();
    updateVisualization();
    updateStepTrace();
}

QVariant HashMapVisualization::convertStringToVariant(const QString &str, HashMap::DataType type)
{
    bool ok;
    switch (type) {
        case HashMap::STRING:
            return QVariant(str);
        case HashMap::INTEGER: {
            int intVal = str.toInt(&ok);
            return ok ? QVariant(intVal) : QVariant();
        }
        case HashMap::DOUBLE: {
            double doubleVal = str.toDouble(&ok);
            return ok ? QVariant(doubleVal) : QVariant();
        }
        case HashMap::FLOAT: {
            float floatVal = str.toFloat(&ok);
            return ok ? QVariant(floatVal) : QVariant();
        }
        default:
            return QVariant();
    }
}

void HashMapVisualization::onInsertClicked()
{
    const QString keyStr = keyInput->text().trimmed();
    const QString valueStr = valueInput->text().trimmed();
    
    if (keyStr.isEmpty() || valueStr.isEmpty()) {
        // Visual feedback instead of popup
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    // Convert strings to appropriate QVariant types
    QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());
    QVariant value = convertStringToVariant(valueStr, hashMap->getValueType());
    
    if (!key.isValid() || !value.isValid()) {
        // Type conversion failed - show error
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    hashMap->put(key, value);
    animateOperation("Insert");
    
    // Clear inputs
    keyInput->clear();
    valueInput->clear();
}

void HashMapVisualization::onSearchClicked()
{
    const QString keyStr = keyInput->text().trimmed();
    const QString valueStr = valueInput->text().trimmed();
    
    // Determine search mode based on which field is filled
    if (keyStr.isEmpty() && valueStr.isEmpty()) {
        // Both empty - show error
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    if (!keyStr.isEmpty()) {
        // Search by key (key field is filled)
        QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());
        
        if (!key.isValid()) {
            keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
            QTimer::singleShot(2000, [this]() {
                keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            });
            return;
        }
        
        // Perform search by key
        auto result = hashMap->get(key);
        animateOperation("Search");
        
        // If found, show the value in the history
        if (result.has_value()) {
            QString foundValue = HashMap::variantToDisplayString(result.value());
            hashMap->addStepToHistory(QString("✅ Found! Key '%1' → Value '%2'").arg(keyStr, foundValue));
            updateStepTrace();
        }
        
        // Animate the search by highlighting the target bucket
        animateSearchResult(keyStr, result.has_value());
        
    } else {
        // Search by value (only value field is filled, key is empty)
        QVariant value = convertStringToVariant(valueStr, hashMap->getValueType());
        
        if (!value.isValid()) {
            valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
            QTimer::singleShot(2000, [this]() {
                valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            });
            return;
        }
        
        // Perform search by value
        auto result = hashMap->findByValue(value);
        animateOperation("Search by Value");
        updateStepTrace();
        
        // No bucket highlighting for value search (searches all buckets)
    }
}

void HashMapVisualization::animateSearchResult(const QString &key, bool found)
{
    // Convert key to QVariant for proper hashing
    QVariant keyVariant = convertStringToVariant(key, hashMap->getKeyType());
    if (!keyVariant.isValid()) return;
    
    // Calculate which bucket the key would be in using HashMap's indexFor method
    const int bucketIndex = hashMap->indexFor(keyVariant, 10);
    
    // Step 1: Show hash calculation (like Binary Tree's step-by-step approach)
    hashMap->addStepToHistory(QString("🔍 Searching for key: %1").arg(key));
    hashMap->addStepToHistory(QString("📊 Calculating hash for key..."));
    updateStepTrace();
    
    QTimer::singleShot(800, [this, key, bucketIndex, found]() {
        // Step 2: Show which bucket to check
        hashMap->addStepToHistory(QString("🎯 Hash points to bucket %1").arg(bucketIndex));
        updateStepTrace();
        
        // Clear any existing highlights
        if (highlightRect) {
            scene->removeItem(highlightRect);
            delete highlightRect;
            highlightRect = nullptr;
        }
        
        // Create highlight rectangle for the target bucket
        const int bucketCount = 10;
        const int totalWidth = bucketCount * (BUCKET_WIDTH + BUCKET_SPACING) - BUCKET_SPACING;
        const int startX = -totalWidth / 2;
        const int x = startX + bucketIndex * (BUCKET_WIDTH + BUCKET_SPACING);
        const int y = 0;
        
        // Calculate bucket height (same logic as drawBuckets)
        const QVector<QVector<QPair<QVariant, QVariant>>> bucketContents = hashMap->getBucketContents();
        int bucketHeight = BUCKET_HEIGHT;
        if (bucketIndex < bucketContents.size() && !bucketContents[bucketIndex].isEmpty()) {
            bucketHeight = BUCKET_HEIGHT + (bucketContents[bucketIndex].size() * 30);
        }
        
        // Create highlight effect (like Binary Tree node highlighting)
        highlightRect = scene->addRect(x - 3, y - 3, BUCKET_WIDTH + 6, bucketHeight + 6,
                                      QPen(found ? QColor(40, 167, 69, 200) : QColor(220, 53, 69, 200), 4),
                                      QBrush(Qt::transparent));
        highlightRect->setZValue(10);
        
        QTimer::singleShot(800, [this, key, found]() {
            // Step 3: Show search result
            if (found) {
                hashMap->addStepToHistory(QString("✅ Key '%1' found in bucket!").arg(key));
            } else {
                hashMap->addStepToHistory(QString("❌ Key '%1' not found in bucket").arg(key));
            }
            updateStepTrace();
            
            // Step 4: Fade out highlight after showing result (like Binary Tree)
            QTimer::singleShot(1200, [this]() {
                if (highlightRect) {
                    scene->removeItem(highlightRect);
                    delete highlightRect;
                    highlightRect = nullptr;
                }
            });
        });
    });
}

void HashMapVisualization::onDeleteClicked()
{
    const QString keyStr = keyInput->text().trimmed();
    
    if (keyStr.isEmpty()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    // Convert string to appropriate QVariant type
    QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());
    
    if (!key.isValid()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    bool removed = hashMap->erase(key);
    animateOperation("Delete");
    
    // Animate the deletion result
    animateSearchResult(keyStr, removed);
    
    keyInput->clear();
}

void HashMapVisualization::onClearClicked()
{
    hashMap->clear();
    animateOperation("Clear");
    // Visual feedback - flash the entire visualization
    QPropertyAnimation *flashAnimation = new QPropertyAnimation(visualizationView, "opacity");
    flashAnimation->setDuration(300);
    flashAnimation->setStartValue(1.0);
    flashAnimation->setEndValue(0.3);
    flashAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    connect(flashAnimation, &QPropertyAnimation::finished, [this, flashAnimation]() {
        QPropertyAnimation *restoreAnimation = new QPropertyAnimation(visualizationView, "opacity");
        restoreAnimation->setDuration(300);
        restoreAnimation->setStartValue(0.3);
        restoreAnimation->setEndValue(1.0);
        restoreAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        restoreAnimation->start();
        flashAnimation->deleteLater();
    });
    
    flashAnimation->start();
}

void HashMapVisualization::onRandomizeClicked()
{
    // Generate random data based on selected types
    HashMap::DataType keyType = hashMap->getKeyType();
    HashMap::DataType valueType = hashMap->getValueType();
    
    for (int i = 0; i < 5; ++i) {
        QVariant key, value;
        
        // Generate random key based on type
        switch (keyType) {
            case HashMap::STRING: {
                QStringList keys = {"apple", "banana", "cherry", "date", "fig", "grape", "kiwi", "lemon"};
                key = keys[QRandomGenerator::global()->bounded(keys.size())];
                break;
            }
            case HashMap::INTEGER:
                key = QRandomGenerator::global()->bounded(1, 100);
                break;
            case HashMap::DOUBLE:
                key = QRandomGenerator::global()->generateDouble() * 100.0;
                break;
            case HashMap::FLOAT:
                key = static_cast<float>(QRandomGenerator::global()->generateDouble() * 100.0f);
                break;
        }
        
        // Generate random value based on type
        switch (valueType) {
            case HashMap::STRING: {
                QStringList values = {"red", "blue", "green", "yellow", "purple", "orange", "pink", "brown"};
                value = values[QRandomGenerator::global()->bounded(values.size())];
                break;
            }
            case HashMap::INTEGER:
                value = QRandomGenerator::global()->bounded(1, 1000);
                break;
            case HashMap::DOUBLE:
                value = QRandomGenerator::global()->generateDouble() * 1000.0;
                break;
            case HashMap::FLOAT:
                value = static_cast<float>(QRandomGenerator::global()->generateDouble() * 1000.0f);
                break;
        }
        
        hashMap->put(key, value);
    }
    
    animateOperation("Randomize");
}

void HashMapVisualization::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Create beautiful gradient background matching the Binary Tree style
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));    // Very light lavender
    gradient.setColorAt(0.5, QColor(242, 235, 255));    // Soft lavender
    gradient.setColorAt(1.0, QColor(237, 228, 255));    // Lavender
    
    painter.fillRect(rect(), gradient);
    
    // Add subtle circular gradients for depth (matching Binary Tree)
    QRadialGradient topCircle(width() * 0.2, height() * 0.15, width() * 0.4);
    topCircle.setColorAt(0.0, QColor(200, 180, 255, 30));
    topCircle.setColorAt(1.0, QColor(200, 180, 255, 0));
    painter.fillRect(rect(), topCircle);
    
    QRadialGradient bottomCircle(width() * 0.8, height() * 0.85, width() * 0.5);
    bottomCircle.setColorAt(0.0, QColor(180, 150, 255, 25));
    bottomCircle.setColorAt(1.0, QColor(180, 150, 255, 0));
    painter.fillRect(rect(), bottomCircle);
}
