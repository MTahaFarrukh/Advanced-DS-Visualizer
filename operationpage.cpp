#include "operationpage.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>

OperationPage::OperationPage(const QString &dataStructureName, QWidget *parent)
    : QWidget(parent)
    , dsName(dataStructureName)
    , animationsStarted(false)
{
    setupUI();
    setupAnimations();
    setMinimumSize(900, 750);
}

OperationPage::~OperationPage()
{
}

void OperationPage::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 25, 50, 40);
    mainLayout->setSpacing(0);

    // Create content container
    contentContainer = new QWidget(this);
    QVBoxLayout *containerLayout = new QVBoxLayout(contentContainer);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // Back button
    backButton = new QPushButton("← Back to Menu", contentContainer);
    backButton->setCursor(Qt::PointingHandCursor);
    styleBackButton(backButton);
    containerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    containerLayout->addSpacing(25);

    // Title
    titleLabel = new QLabel(dsName, contentContainer);
    QFont titleFont;

    QStringList preferredFonts = {"Segoe UI", "Poppins", "SF Pro Display", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }

    titleFont.setPointSize(38);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, -1);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; background: transparent; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);

    containerLayout->addWidget(titleLabel);
    containerLayout->addSpacing(12);

    // Subtitle
    subtitleLabel = new QLabel("What would you like to learn?", contentContainer);
    QFont subtitleFont;
    subtitleFont.setFamily(titleFont.family());
    subtitleFont.setPointSize(15);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setStyleSheet("color: #6b5b95; background: transparent; border: none;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    containerLayout->addWidget(subtitleLabel);
    containerLayout->addSpacing(50);

    // Create grid layout for operation cards (2x2)
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(34);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Create operation cards
    QWidget *insertCard = createOperationCard(
        "Insertion",
        "Learn how to add elements",
        "➕",
        insertButton
        );

    QWidget *deleteCard = createOperationCard(
        "Deletion",
        "Learn how to remove elements",
        "➖",
        deleteButton
        );

    QWidget *searchCard = createOperationCard(
        "Search",
        "Learn how to find elements",
        "🔍",
        searchButton
        );

    QWidget *traversalCard = createOperationCard(
        "Traversal",
        "Learn how to visit all elements",
        "🔄",
        traversalButton
        );

    // Add cards to grid
    gridLayout->addWidget(insertCard, 0, 0);
    gridLayout->addWidget(deleteCard, 0, 1);
    gridLayout->addWidget(searchCard, 1, 0);
    gridLayout->addWidget(traversalCard, 1, 1);

    // Store cards for animation
    cards.append(insertCard);
    cards.append(deleteCard);
    cards.append(searchCard);
    cards.append(traversalCard);

    containerLayout->addLayout(gridLayout);
    containerLayout->addStretch();

    contentContainer->setLayout(containerLayout);
    mainLayout->addWidget(contentContainer);

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &OperationPage::onBackClicked);
    connect(insertButton, &QPushButton::clicked, this, &OperationPage::onInsertionClicked);
    connect(deleteButton, &QPushButton::clicked, this, &OperationPage::onDeletionClicked);
    connect(searchButton, &QPushButton::clicked, this, &OperationPage::onSearchClicked);
    connect(traversalButton, &QPushButton::clicked, this, &OperationPage::onTraversalClicked);

    setLayout(mainLayout);
}

QWidget* OperationPage::createOperationCard(const QString &title, const QString &description,
                                            const QString &icon, QPushButton* &button)
{
    QWidget *card = new QWidget(this);
    card->setFixedSize(380, 245);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 20, 24, 20);
    cardLayout->setSpacing(5);

    // Icon label
    QLabel *iconLabel = new QLabel(icon, card);
    QFont iconFont;
    iconFont.setPointSize(42);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedHeight(54);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    cardLayout->addWidget(iconLabel);

    cardLayout->addSpacing(0);

    // Title label
    QLabel *titleLabel = new QLabel(title, card);
    QFont titleFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; margin-top: 2px; background: transparent; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(32);
    cardLayout->addWidget(titleLabel);

    // Description label
    QLabel *descLabel = new QLabel(description, card);
    QFont descFont;
    descFont.setFamily(titleFont.family());
    descFont.setPointSize(11);
    descLabel->setFont(descFont);
    descLabel->setStyleSheet("color: #7a6b95; background: transparent; border: none;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setFixedHeight(36);
    cardLayout->addWidget(descLabel);

    cardLayout->addSpacing(3);

    // Button
    button = new QPushButton("Learn", card);
    button->setFixedHeight(45);
    button->setCursor(Qt::PointingHandCursor);
    styleButton(button);
    cardLayout->addWidget(button);

    card->setLayout(cardLayout);
    styleCard(card);

    return card;
}

void OperationPage::styleCard(QWidget *card)
{
    card->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.95);
            border-radius: 24px;
            border: 2px solid rgba(123, 79, 255, 0.1);
        }
    )");

    // Add drop shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(8);
    shadow->setColor(QColor(123, 79, 255, 40));
    card->setGraphicsEffect(shadow);
}

void OperationPage::styleButton(QPushButton *button)
{
    QFont buttonFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            buttonFont.setFamily(fontName);
            break;
        }
    }

    buttonFont.setPointSize(13);
    buttonFont.setBold(true);
    button->setFont(buttonFont);

    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 22px;
            padding: 12px;
            text-align: center;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a32cc, stop:1 #7a4ccc);
        }
    )";

    button->setStyleSheet(buttonStyle);
}

void OperationPage::styleBackButton(QPushButton *button)
{
    button->setFixedSize(140, 38);

    QFont buttonFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            buttonFont.setFamily(fontName);
            break;
        }
    }

    buttonFont.setPointSize(12);
    buttonFont.setBold(false);
    button->setFont(buttonFont);

    QString buttonStyle = R"(
        QPushButton {
            background-color: rgba(123, 79, 255, 0.1);
            color: #7b4fff;
            border: 2px solid #7b4fff;
            border-radius: 19px;
            padding: 8px 16px;
            text-align: center;
        }
        QPushButton:hover {
            background-color: rgba(123, 79, 255, 0.2);
            border-color: #6c3cff;
            color: #6c3cff;
        }
        QPushButton:pressed {
            background-color: rgba(123, 79, 255, 0.3);
        }
    )";

    button->setStyleSheet(buttonStyle);
}

void OperationPage::setupAnimations()
{
    animationGroup = new QParallelAnimationGroup(this);

    // Create fade-in animations for each card with stagger effect
    for (int i = 0; i < cards.size(); ++i) {
        QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(this);
        opacity->setOpacity(0.0);
        cardOpacities.append(opacity);

        // Create a sequential group for this card (pause + fade)
        QSequentialAnimationGroup *cardSequence = new QSequentialAnimationGroup();

        // Add pause before animation starts (for stagger effect)
        QPauseAnimation *pause = new QPauseAnimation(i * 100);
        cardSequence->addAnimation(pause);

        // Add fade-in animation
        QPropertyAnimation *fadeIn = new QPropertyAnimation(opacity, "opacity");
        fadeIn->setDuration(800);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        cardSequence->addAnimation(fadeIn);

        animationGroup->addAnimation(cardSequence);
    }
}

void OperationPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Create gradient background
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));
    gradient.setColorAt(0.5, QColor(242, 235, 255));
    gradient.setColorAt(1.0, QColor(237, 228, 255));

    painter.fillRect(rect(), gradient);

    // Add subtle circular gradients for depth
    QRadialGradient topCircle(width() * 0.2, height() * 0.15, width() * 0.4);
    topCircle.setColorAt(0.0, QColor(200, 180, 255, 30));
    topCircle.setColorAt(1.0, QColor(200, 180, 255, 0));
    painter.fillRect(rect(), topCircle);

    QRadialGradient bottomCircle(width() * 0.8, height() * 0.85, width() * 0.5);
    bottomCircle.setColorAt(0.0, QColor(180, 150, 255, 25));
    bottomCircle.setColorAt(1.0, QColor(180, 150, 255, 0));
    painter.fillRect(rect(), bottomCircle);
}

void OperationPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!animationsStarted) {
        // Apply opacity effects to cards
        for (int i = 0; i < cards.size() && i < cardOpacities.size(); ++i) {
            cards[i]->setGraphicsEffect(cardOpacities[i]);
        }

        // Start staggered animations
        animationGroup->start();
        animationsStarted = true;
    }
}

void OperationPage::onBackClicked()
{
    emit backToMenu();
}

void OperationPage::onInsertionClicked()
{
    qDebug() << dsName << "- Insertion operation selected";
    emit operationSelected("Insertion");
}

void OperationPage::onDeletionClicked()
{
    qDebug() << dsName << "- Deletion operation selected";
    emit operationSelected("Deletion");
    // TODO: Open visualization window for deletion
}

void OperationPage::onSearchClicked()
{
    qDebug() << dsName << "- Search operation selected";
    emit operationSelected("Search");
    // TODO: Open visualization window for search
}

void OperationPage::onTraversalClicked()
{
    qDebug() << dsName << "- Traversal operation selected";
    emit operationSelected("Traversal");
    // TODO: Open visualization window for traversal
}
