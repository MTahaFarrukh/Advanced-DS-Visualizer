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

HashMapVisualization::HashMapVisualization(QWidget *parent)
    : QWidget(parent)
    , hashMap(new HashMap(8, 0.75f))
    , animationTimer(new QTimer(this))
    , highlightAnimation(nullptr)
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
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 10, 20);
    leftLayout->setSpacing(15);
    
    // Header with back button and title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    backButton = new QPushButton("← Back to Operations");
    backButton->setFixedSize(160, 40);
    backButton->setCursor(Qt::PointingHandCursor);
    styleButton(backButton, "#6c757d");
    connect(backButton, &QPushButton::clicked, this, &HashMapVisualization::onBackClicked);
    
    titleLabel = new QLabel("HashMap Visualization");
    QFont titleFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "SF Pro Display", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();
    
    leftLayout->addLayout(headerLayout);
    
    // Visualization area
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(QBrush(QColor(248, 249, 250)));
    
    visualizationView = new QGraphicsView(scene);
    visualizationView->setRenderHint(QPainter::Antialiasing);
    visualizationView->setDragMode(QGraphicsView::ScrollHandDrag);
    visualizationView->setMinimumHeight(300);
    visualizationView->setStyleSheet(R"(
        QGraphicsView {
            border: 2px solid #e9ecef;
            border-radius: 12px;
            background-color: #f8f9fa;
        }
    )");
    
    leftLayout->addWidget(visualizationView, 1);
    
    setupControlPanel();
}

void HashMapVisualization::setupControlPanel()
{
    controlPanel = new QWidget();
    controlPanel->setFixedHeight(120);
    controlPanel->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.9);
            border-radius: 12px;
            border: 1px solid #e9ecef;
        }
    )");
    
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(20, 15, 20, 15);
    controlLayout->setSpacing(10);
    
    // Input row
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    QLabel *keyLabel = new QLabel("Key:");
    keyLabel->setStyleSheet("color: #495057; font-weight: bold;");
    keyInput = new QLineEdit();
    keyInput->setPlaceholderText("Enter key");
    styleInput(keyInput);
    
    QLabel *valueLabel = new QLabel("Value:");
    valueLabel->setStyleSheet("color: #495057; font-weight: bold;");
    valueInput = new QLineEdit();
    valueInput->setPlaceholderText("Enter value");
    styleInput(valueInput);
    
    inputLayout->addWidget(keyLabel);
    inputLayout->addWidget(keyInput);
    inputLayout->addWidget(valueLabel);
    inputLayout->addWidget(valueInput);
    
    // Button row
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    insertButton = new QPushButton("Insert");
    searchButton = new QPushButton("Search");
    deleteButton = new QPushButton("Delete");
    clearButton = new QPushButton("Clear All");
    randomizeButton = new QPushButton("Randomize");
    
    styleButton(insertButton, "#28a745");
    styleButton(searchButton, "#17a2b8");
    styleButton(deleteButton, "#dc3545");
    styleButton(clearButton, "#6c757d");
    styleButton(randomizeButton, "#fd7e14");
    
    buttonLayout->addWidget(insertButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(randomizeButton);
    
    // Stats row
    QHBoxLayout *statsLayout = new QHBoxLayout();
    
    sizeLabel = new QLabel("Size: 0");
    bucketCountLabel = new QLabel("Buckets: 8");
    loadFactorLabel = new QLabel("Load Factor: 0.00");
    
    QString statsStyle = "color: #495057; font-weight: bold; padding: 5px;";
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
    rightPanel->setMinimumWidth(300);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border-left: 2px solid #e9ecef;
        }
    )");
    
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(15, 20, 15, 20);
    rightLayout->setSpacing(10);
    
    // Steps title
    stepsTitle = new QLabel("Step-by-Step Trace");
    QFont stepsFont;
    stepsFont.setFamily("Segoe UI");
    stepsFont.setPointSize(16);
    stepsFont.setBold(true);
    stepsTitle->setFont(stepsFont);
    stepsTitle->setStyleSheet("color: #2d1b69; padding-bottom: 10px;");
    stepsTitle->setAlignment(Qt::AlignCenter);
    
    // Steps list
    stepsList = new QListWidget();
    stepsList->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 5px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #f1f3f4;
            color: #495057;
        }
        QListWidget::item:selected {
            background-color: #e3f2fd;
            color: #1976d2;
        }
    )");
    
    rightLayout->addWidget(stepsTitle);
    rightLayout->addWidget(stepsList, 1);
}

void HashMapVisualization::styleButton(QPushButton *button, const QString &color)
{
    button->setFixedHeight(35);
    button->setCursor(Qt::PointingHandCursor);
    
    QFont buttonFont;
    buttonFont.setFamily("Segoe UI");
    buttonFont.setPointSize(11);
    buttonFont.setBold(true);
    button->setFont(buttonFont);
    
    QString buttonStyle = QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
    )").arg(color)
       .arg(color == "#7b4fff" ? "#6c3cff" : color)
       .arg(color == "#7b4fff" ? "#5a32cc" : color);
    
    button->setStyleSheet(buttonStyle);
}

void HashMapVisualization::styleInput(QLineEdit *input)
{
    input->setFixedHeight(35);
    input->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e9ecef;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 12px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #7b4fff;
            outline: none;
        }
    )");
}

void HashMapVisualization::drawBuckets()
{
    // Clear existing visualization
    scene->clear();
    bucketRects.clear();
    bucketTexts.clear();
    chainTexts.clear();
    
    const int bucketCount = hashMap->bucketCount();
    const QVector<int> bucketSizes = hashMap->bucketSizes();
    
    // Calculate layout
    const int bucketsPerRow = std::min(MAX_VISIBLE_BUCKETS, bucketCount);
    const int rows = (bucketCount + bucketsPerRow - 1) / bucketsPerRow;
    
    const int totalWidth = bucketsPerRow * (BUCKET_WIDTH + BUCKET_SPACING) - BUCKET_SPACING;
    const int startX = -totalWidth / 2;
    
    bucketRects.resize(bucketCount);
    bucketTexts.resize(bucketCount);
    chainTexts.resize(bucketCount);
    
    for (int i = 0; i < bucketCount; ++i) {
        const int row = i / bucketsPerRow;
        const int col = i % bucketsPerRow;
        
        const int x = startX + col * (BUCKET_WIDTH + BUCKET_SPACING);
        const int y = row * (BUCKET_HEIGHT + 80); // Extra space for chains
        
        // Draw bucket rectangle
        QGraphicsRectItem *bucketRect = scene->addRect(
            x, y, BUCKET_WIDTH, BUCKET_HEIGHT,
            QPen(QColor(52, 58, 64), 2),
            QBrush(QColor(255, 255, 255))
        );
        bucketRects[i] = bucketRect;
        
        // Bucket index label
        QGraphicsTextItem *indexText = scene->addText(QString::number(i));
        indexText->setPos(x + BUCKET_WIDTH/2 - 8, y - 25);
        indexText->setDefaultTextColor(QColor(73, 80, 87));
        QFont indexFont = indexText->font();
        indexFont.setBold(true);
        indexText->setFont(indexFont);
        
        // Bucket size label
        QGraphicsTextItem *sizeText = scene->addText(QString("(%1)").arg(bucketSizes[i]));
        sizeText->setPos(x + BUCKET_WIDTH/2 - 10, y + BUCKET_HEIGHT + 5);
        sizeText->setDefaultTextColor(QColor(108, 117, 125));
        bucketTexts[i] = sizeText;
        
        // Draw chain items (placeholder - would need actual key-value pairs)
        QVector<QGraphicsTextItem*> chainItems;
        for (int j = 0; j < bucketSizes[i]; ++j) {
            const int chainY = y + BUCKET_HEIGHT + 30 + j * CHAIN_ITEM_HEIGHT;
            QGraphicsTextItem *chainItem = scene->addText(QString("Item %1").arg(j + 1));
            chainItem->setPos(x + 5, chainY);
            chainItem->setDefaultTextColor(QColor(40, 167, 69));
            QFont chainFont = chainItem->font();
            chainFont.setPointSize(9);
            chainItem->setFont(chainFont);
            chainItems.append(chainItem);
        }
        chainTexts[i] = chainItems;
    }
    
    // Adjust scene rect
    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50, -50, 50, 50));
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
    bucketCountLabel->setText(QString("Buckets: %1").arg(hashMap->bucketCount()));
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
        QMessageBox::warning(this, "Input Error", "Please enter both key and value.");
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
        QMessageBox::warning(this, "Input Error", "Please enter a key to search.");
        return;
    }
    
    auto result = hashMap->get(key);
    animateOperation("Search");
    
    if (result.has_value()) {
        QMessageBox::information(this, "Search Result", 
                                QString("Found: %1 → %2").arg(key, result.value()));
    } else {
        QMessageBox::information(this, "Search Result", 
                                QString("Key '%1' not found.").arg(key));
    }
}

void HashMapVisualization::onDeleteClicked()
{
    const QString key = keyInput->text().trimmed();
    
    if (key.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a key to delete.");
        return;
    }
    
    bool removed = hashMap->erase(key);
    animateOperation("Delete");
    
    if (removed) {
        QMessageBox::information(this, "Delete Result", 
                                QString("Key '%1' deleted successfully.").arg(key));
    } else {
        QMessageBox::information(this, "Delete Result", 
                                QString("Key '%1' not found.").arg(key));
    }
    
    keyInput->clear();
}

void HashMapVisualization::onClearClicked()
{
    hashMap->clear();
    animateOperation("Clear");
    QMessageBox::information(this, "Clear", "HashMap cleared successfully.");
}

void HashMapVisualization::onRandomizeClicked()
{
    // Add some random key-value pairs
    QStringList sampleKeys = {"apple", "banana", "cherry", "date", "elderberry", 
                             "fig", "grape", "honeydew", "kiwi", "lemon"};
    QStringList sampleValues = {"red", "yellow", "dark red", "brown", "purple",
                               "green", "purple", "green", "brown", "yellow"};
    
    for (int i = 0; i < 6; ++i) {
        const int idx = QRandomGenerator::global()->bounded(sampleKeys.size());
        hashMap->put(sampleKeys[idx], sampleValues[idx]);
    }
    
    animateOperation("Randomize");
    QMessageBox::information(this, "Randomize", "Added random key-value pairs.");
}
