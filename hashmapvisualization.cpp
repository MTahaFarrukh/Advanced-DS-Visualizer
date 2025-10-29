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
    // Main splitter for left (visualization) and right (steps) panels
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
    setupStepTracePanel();
    
    // Set splitter proportions (70% visualization, 30% steps)
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({840, 360});
    
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
            background-color: rgba(123, 79, 255, 0.1);
            color: #7b4fff;
            border: 2px solid rgba(123, 79, 255, 0.3);
            border-radius: 22px;
            padding: 10px 20px;
            font-family: 'Segoe UI';
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(123, 79, 255, 0.2);
            border-color: #7b4fff;
        }
        QPushButton:pressed {
            background-color: rgba(123, 79, 255, 0.3);
        }
    )");
    connect(backButton, &QPushButton::clicked, this, &HashMapVisualization::onBackClicked);
    
    titleLabel = new QLabel("Hash Table Visualization");
    QFont titleFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "SF Pro Display", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }
    titleFont.setPointSize(32);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();
    
    leftLayout->addLayout(headerLayout);
    
    // Visualization area with gradient background
    scene = new QGraphicsScene(this);
    
    visualizationView = new QGraphicsView(scene);
    visualizationView->setRenderHint(QPainter::Antialiasing);
    visualizationView->setDragMode(QGraphicsView::ScrollHandDrag);
    visualizationView->setMinimumHeight(400);
    visualizationView->setStyleSheet(R"(
        QGraphicsView {
            border: none;
            border-radius: 20px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(250, 248, 255, 0.95),
                stop:1 rgba(245, 240, 255, 0.95));
        }
    )");
    
    // Add drop shadow to visualization area
    QGraphicsDropShadowEffect *viewShadow = new QGraphicsDropShadowEffect();
    viewShadow->setBlurRadius(25);
    viewShadow->setXOffset(0);
    viewShadow->setYOffset(8);
    viewShadow->setColor(QColor(123, 79, 255, 30));
    visualizationView->setGraphicsEffect(viewShadow);
    
    leftLayout->addWidget(visualizationView, 1);
    
    setupControlPanel();
}

void HashMapVisualization::setupControlPanel()
{
    controlPanel = new QWidget();
    controlPanel->setFixedHeight(170);
    controlPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 0.95),
                stop:1 rgba(250, 248, 255, 0.95));
            border-radius: 20px;
            border: 2px solid rgba(123, 79, 255, 0.1);
        }
    )");
    
    // Add drop shadow to control panel
    QGraphicsDropShadowEffect *controlShadow = new QGraphicsDropShadowEffect();
    controlShadow->setBlurRadius(20);
    controlShadow->setXOffset(0);
    controlShadow->setYOffset(6);
    controlShadow->setColor(QColor(123, 79, 255, 25));
    controlPanel->setGraphicsEffect(controlShadow);
    
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(30, 20, 30, 20);
    controlLayout->setSpacing(12);
    
    // Input row with modern styling
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(15);
    
    keyInput = new QLineEdit();
    keyInput->setPlaceholderText("Enter key (e.g., apple)");
    keyInput->setFixedHeight(45);
    keyInput->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid rgba(123, 79, 255, 0.2);
            border-radius: 22px;
            padding: 12px 20px;
            font-size: 14px;
            font-family: 'Segoe UI';
            background-color: rgba(255, 255, 255, 0.8);
            color: #2d1b69;
        }
        QLineEdit:focus {
            border-color: #7b4fff;
            background-color: white;
            outline: none;
        }
        QLineEdit::placeholder {
            color: rgba(45, 27, 105, 0.5);
        }
    )");
    
    valueInput = new QLineEdit();
    valueInput->setPlaceholderText("Enter value (e.g., red)");
    valueInput->setFixedHeight(45);
    valueInput->setStyleSheet(keyInput->styleSheet());
    
    inputLayout->addWidget(keyInput);
    inputLayout->addWidget(valueInput);
    
    // Button row with beautiful styling
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    insertButton = new QPushButton("Insert");
    insertButton->setMinimumWidth(100);
    insertButton->setMaximumWidth(150);
    
    searchButton = new QPushButton("Search");
    searchButton->setMinimumWidth(100);
    searchButton->setMaximumWidth(150);
    
    deleteButton = new QPushButton("Delete");
    deleteButton->setMinimumWidth(100);
    deleteButton->setMaximumWidth(150);
    
    clearButton = new QPushButton("Clear");
    clearButton->setMinimumWidth(100);
    clearButton->setMaximumWidth(150);
    
    randomizeButton = new QPushButton("Random");
    randomizeButton->setMinimumWidth(100);
    randomizeButton->setMaximumWidth(150);
    
    // Style buttons with modern gradients
    QString insertStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #28a745, stop:1 #20c997);
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 16px;
            font-family: 'Segoe UI';
            font-size: 12px;
            font-weight: bold;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #218838, stop:1 #1ea085);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #1e7e34, stop:1 #198754);
        }
    )";
    
    QString searchStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #17a2b8, stop:1 #6f42c1);
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 16px;
            font-family: 'Segoe UI';
            font-size: 12px;
            font-weight: bold;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #138496, stop:1 #5a2d91);
        }
    )";
    
    QString deleteStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #dc3545, stop:1 #e83e8c);
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 16px;
            font-family: 'Segoe UI';
            font-size: 12px;
            font-weight: bold;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #c82333, stop:1 #d91a72);
        }
    )";
    
    QString clearStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c757d, stop:1 #495057);
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 16px;
            font-family: 'Segoe UI';
            font-size: 12px;
            font-weight: bold;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a6268, stop:1 #3d4142);
        }
    )";
    
    QString randomizeStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #fd7e14, stop:1 #ffc107);
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 16px;
            font-family: 'Segoe UI';
            font-size: 12px;
            font-weight: bold;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #e8650e, stop:1 #e0a800);
        }
    )";
    
    insertButton->setStyleSheet(insertStyle);
    searchButton->setStyleSheet(searchStyle);
    deleteButton->setStyleSheet(deleteStyle);
    clearButton->setStyleSheet(clearStyle);
    randomizeButton->setStyleSheet(randomizeStyle);
    
    insertButton->setCursor(Qt::PointingHandCursor);
    searchButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setCursor(Qt::PointingHandCursor);
    clearButton->setCursor(Qt::PointingHandCursor);
    randomizeButton->setCursor(Qt::PointingHandCursor);
    
    buttonLayout->addWidget(insertButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(randomizeButton);
    
    // Stats row with modern styling
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    sizeLabel = new QLabel("Size: 0");
    bucketCountLabel = new QLabel("Buckets: 8");
    loadFactorLabel = new QLabel("Load Factor: 0.00");
    
    QString statsStyle = R"(
        QLabel {
            color: #2d1b69;
            font-weight: bold;
            font-family: 'Segoe UI';
            font-size: 13px;
            padding: 8px 12px;
            background-color: rgba(123, 79, 255, 0.1);
            border-radius: 12px;
            border: 1px solid rgba(123, 79, 255, 0.2);
        }
    )";
    
    sizeLabel->setStyleSheet(statsStyle);
    bucketCountLabel->setStyleSheet(statsStyle);
    loadFactorLabel->setStyleSheet(statsStyle);
    
    statsLayout->addWidget(sizeLabel);
    statsLayout->addWidget(bucketCountLabel);
    statsLayout->addWidget(loadFactorLabel);
    statsLayout->addStretch();
    
    controlLayout->addLayout(inputLayout);
    controlLayout->addLayout(buttonLayout);
    controlLayout->addLayout(statsLayout);
    
    leftLayout->addWidget(controlPanel);
    
    // Connect signals
    connect(insertButton, &QPushButton::clicked, this, &HashMapVisualization::onInsertClicked);
    connect(searchButton, &QPushButton::clicked, this, &HashMapVisualization::onSearchClicked);
    connect(deleteButton, &QPushButton::clicked, this, &HashMapVisualization::onDeleteClicked);
    connect(clearButton, &QPushButton::clicked, this, &HashMapVisualization::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &HashMapVisualization::onRandomizeClicked);
}

void HashMapVisualization::setupStepTracePanel()
{
    rightPanel = new QWidget();
    rightPanel->setMinimumWidth(350);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(250, 248, 255, 0.98),
                stop:1 rgba(245, 240, 255, 0.98));
            border-left: none;
        }
    )");
    
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(20, 30, 20, 30);
    rightLayout->setSpacing(20);
    
    // Steps title with modern styling
    stepsTitle = new QLabel("Step-by-Step Trace");
    QFont stepsFont;
    stepsFont.setFamily("Segoe UI");
    stepsFont.setPointSize(18);
    stepsFont.setBold(true);
    stepsTitle->setFont(stepsFont);
    stepsTitle->setStyleSheet(R"(
        QLabel {
            color: #2d1b69;
            padding: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(123, 79, 255, 0.1),
                stop:1 rgba(123, 79, 255, 0.05));
            border-radius: 15px;
            border: 2px solid rgba(123, 79, 255, 0.2);
        }
    )");
    stepsTitle->setAlignment(Qt::AlignCenter);
    
    // Add drop shadow to title
    QGraphicsDropShadowEffect *titleShadow = new QGraphicsDropShadowEffect();
    titleShadow->setBlurRadius(15);
    titleShadow->setXOffset(0);
    titleShadow->setYOffset(4);
    titleShadow->setColor(QColor(123, 79, 255, 20));
    stepsTitle->setGraphicsEffect(titleShadow);
    
    // Steps list with beautiful styling
    stepsList = new QListWidget();
    stepsList->setStyleSheet(R"(
        QListWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 0.95),
                stop:1 rgba(250, 248, 255, 0.95));
            border: 2px solid rgba(123, 79, 255, 0.1);
            border-radius: 15px;
            padding: 10px;
            font-family: 'Segoe UI';
            font-size: 13px;
            font-weight: 500;
        }
        QListWidget::item {
            padding: 12px 15px;
            margin: 3px 0px;
            border-radius: 10px;
            background-color: rgba(255, 255, 255, 0.7);
            border: 1px solid rgba(123, 79, 255, 0.1);
            color: #2d1b69;
            line-height: 1.4;
        }
        QListWidget::item:hover {
            background-color: rgba(123, 79, 255, 0.05);
            border-color: rgba(123, 79, 255, 0.2);
        }
        QListWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(123, 79, 255, 0.15),
                stop:1 rgba(123, 79, 255, 0.1));
            color: #2d1b69;
            border-color: #7b4fff;
            font-weight: bold;
        }
        QScrollBar:vertical {
            background-color: rgba(123, 79, 255, 0.1);
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: rgba(123, 79, 255, 0.3);
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgba(123, 79, 255, 0.5);
        }
    )");
    
    // Add drop shadow to steps list
    QGraphicsDropShadowEffect *listShadow = new QGraphicsDropShadowEffect();
    listShadow->setBlurRadius(20);
    listShadow->setXOffset(0);
    listShadow->setYOffset(6);
    listShadow->setColor(QColor(123, 79, 255, 15));
    stepsList->setGraphicsEffect(listShadow);
    
    rightLayout->addWidget(stepsTitle);
    rightLayout->addWidget(stepsList, 1);
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
    const QVector<QVector<QPair<QString, QString>>> bucketContents = hashMap->getBucketContents();
    
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
                const QString &key = items[j].first;
                const QString &value = items[j].second;
                QString displayText = QString("%1→%2").arg(key.left(4), value.left(4));
                
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
    
    // Add title and limitation note
    QGraphicsTextItem *vizTitle = scene->addText("Hash Table (Open Chaining)");
    QFont titleFont("Segoe UI", 18);
    titleFont.setBold(true);
    vizTitle->setFont(titleFont);
    vizTitle->setDefaultTextColor(QColor(45, 27, 105));
    vizTitle->setPos(-totalWidth/2, -85);
    
    // Add limitation note
    QGraphicsTextItem *limitNote = scene->addText("* Limited to 10 buckets for screen visibility");
    QFont noteFont("Segoe UI", 10);
    noteFont.setItalic(true);
    limitNote->setFont(noteFont);
    limitNote->setDefaultTextColor(QColor(108, 117, 125));
    limitNote->setPos(-totalWidth/2, -60);
    
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
    
    for (const QString &step : steps) {
        stepsList->addItem(step);
    }
    
    // Auto-scroll to bottom
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

void HashMapVisualization::onInsertClicked()
{
    const QString key = keyInput->text().trimmed();
    const QString value = valueInput->text().trimmed();
    
    if (key.isEmpty() || value.isEmpty()) {
        // Visual feedback instead of popup
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
    const QString key = keyInput->text().trimmed();
    
    if (key.isEmpty()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    // Perform search and get result
    auto result = hashMap->get(key);
    animateOperation("Search");
    
    // Animate the search by highlighting the target bucket
    animateSearchResult(key, result.has_value());
}

void HashMapVisualization::animateSearchResult(const QString &key, bool found)
{
    // Calculate which bucket the key would be in
    const size_t hash = static_cast<size_t>(qHash(key));
    const int bucketIndex = static_cast<int>(hash % 10);
    
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
    const QVector<QVector<QPair<QString, QString>>> bucketContents = hashMap->getBucketContents();
    int bucketHeight = BUCKET_HEIGHT;
    if (bucketIndex < bucketContents.size() && !bucketContents[bucketIndex].isEmpty()) {
        bucketHeight = BUCKET_HEIGHT + (bucketContents[bucketIndex].size() * 30);
    }
    
    // Create highlight effect
    highlightRect = scene->addRect(x - 3, y - 3, BUCKET_WIDTH + 6, bucketHeight + 6,
                                  QPen(found ? QColor(40, 167, 69, 200) : QColor(220, 53, 69, 200), 4),
                                  QBrush(Qt::transparent));
    highlightRect->setZValue(10);
    
    // Use a timer for highlight fade instead of QPropertyAnimation on QGraphicsItem
    QTimer::singleShot(2000, [this]() {
        if (highlightRect) {
            scene->removeItem(highlightRect);
            delete highlightRect;
            highlightRect = nullptr;
        }
    });
}

void HashMapVisualization::onDeleteClicked()
{
    const QString key = keyInput->text().trimmed();
    
    if (key.isEmpty()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }
    
    bool removed = hashMap->erase(key);
    animateOperation("Delete");
    
    // Animate the deletion result
    animateSearchResult(key, removed);
    
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
    // Add some random key-value pairs with consistent types
    QStringList sampleKeys = {"apple", "banana", "cherry", "date", "fig", "grape"};
    QStringList sampleValues = {"red", "yellow", "dark", "brown", "green", "purple"};
    
    for (int i = 0; i < 5; ++i) {
        const int idx = QRandomGenerator::global()->bounded(sampleKeys.size());
        hashMap->put(sampleKeys[idx], sampleValues[idx]);
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
