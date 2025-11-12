#include "treedeletion.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QFontDatabase>
#include <QMessageBox>
#include <QDebug>
#include <cmath>

TreeDeletion::TreeDeletion(QWidget *parent)
    : QWidget(parent)
    , root(nullptr)
    , currentSearchStep(0)
    , isAnimating(false)
{
    setupUI();
    setMinimumSize(900, 750);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });
}

TreeDeletion::~TreeDeletion()
{
    clearTree(root);
}

void TreeDeletion::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Top section - Header and controls
    QVBoxLayout *topSection = new QVBoxLayout();
    topSection->setSpacing(15);

    // Back button
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

    // Title
    titleLabel = new QLabel("Binary Tree - Deletion", this);
    QFont titleFont("Segoe UI", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69;");
    titleLabel->setAlignment(Qt::AlignCenter);
    topSection->addWidget(titleLabel);

    // Instruction label
    instructionLabel = new QLabel("Insert values to build a tree, then delete nodes", this);
    QFont instructionFont("Segoe UI", 12);
    instructionLabel->setFont(instructionFont);
    instructionLabel->setStyleSheet("color: #6b5b95;");
    instructionLabel->setAlignment(Qt::AlignCenter);
    topSection->addWidget(instructionLabel);

    mainLayout->addLayout(topSection);

    // Control panel - Insert
    QHBoxLayout *insertLayout = new QHBoxLayout();
    insertLayout->setSpacing(15);
    insertLayout->addStretch();

    QLabel *insertLabel = new QLabel("Insert:", this);
    insertLabel->setStyleSheet("color: #2d1b69; font-size: 13px; font-weight: bold;");
    insertLayout->addWidget(insertLabel);

    insertInputField = new QLineEdit(this);
    insertInputField->setPlaceholderText("Value to insert");
    insertInputField->setFixedSize(150, 40);
    insertInputField->setAlignment(Qt::AlignCenter);
    QFont inputFont("Segoe UI", 12);
    insertInputField->setFont(inputFont);
    insertInputField->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 20px;
            padding: 8px 16px;
            color: #2d1b69;
        }
        QLineEdit:focus {
            border-color: #7b4fff;
        }
    )");

    insertButton = new QPushButton("Insert", this);
    insertButton->setFixedSize(100, 40);
    insertButton->setCursor(Qt::PointingHandCursor);
    QFont btnFont("Segoe UI", 12, QFont::Bold);
    insertButton->setFont(btnFont);
    insertButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #28a745, stop:1 #48c765);
            color: white;
            border: none;
            border-radius: 20px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #218838, stop:1 #38b755);
        }
        QPushButton:disabled {
            background: #cccccc;
        }
    )");

    insertLayout->addWidget(insertInputField);
    insertLayout->addWidget(insertButton);
    insertLayout->addStretch();

    mainLayout->addLayout(insertLayout);

    // Control panel - Delete
    QHBoxLayout *deleteLayout = new QHBoxLayout();
    deleteLayout->setSpacing(15);
    deleteLayout->addStretch();

    QLabel *deleteLabel = new QLabel("Delete:", this);
    deleteLabel->setStyleSheet("color: #2d1b69; font-size: 13px; font-weight: bold;");
    deleteLayout->addWidget(deleteLabel);

    deleteInputField = new QLineEdit(this);
    deleteInputField->setPlaceholderText("Value to delete");
    deleteInputField->setFixedSize(150, 40);
    deleteInputField->setAlignment(Qt::AlignCenter);
    deleteInputField->setFont(inputFont);
    deleteInputField->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 20px;
            padding: 8px 16px;
            color: #2d1b69;
        }
        QLineEdit:focus {
            border-color: #ff4444;
        }
    )");

    deleteButton = new QPushButton("Delete", this);
    deleteButton->setFixedSize(100, 40);
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setFont(btnFont);
    deleteButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #dc3545, stop:1 #ff5565);
            color: white;
            border: none;
            border-radius: 20px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #c82333, stop:1 #ef4555);
        }
        QPushButton:disabled {
            background: #cccccc;
        }
    )");

    clearButton = new QPushButton("Clear Tree", this);
    clearButton->setFixedSize(100, 40);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setFont(btnFont);
    clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 79, 79, 0.9);
            color: white;
            border: none;
            border-radius: 20px;
        }
        QPushButton:hover {
            background-color: rgba(255, 60, 60, 1);
        }
    )");

    deleteLayout->addWidget(deleteInputField);
    deleteLayout->addWidget(deleteButton);
    deleteLayout->addWidget(clearButton);
    deleteLayout->addStretch();

    mainLayout->addLayout(deleteLayout);

    // Status label
    statusLabel = new QLabel("Tree is empty. Start by inserting values!", this);
    QFont statusFont("Segoe UI", 11);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #7b4fff; padding: 8px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    mainLayout->addStretch();

    setLayout(mainLayout);

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &TreeDeletion::onBackClicked);
    connect(insertButton, &QPushButton::clicked, this, &TreeDeletion::onInsertClicked);
    connect(deleteButton, &QPushButton::clicked, this, &TreeDeletion::onDeleteClicked);
    connect(clearButton, &QPushButton::clicked, this, &TreeDeletion::onClearClicked);
    connect(insertInputField, &QLineEdit::returnPressed, this, &TreeDeletion::onInsertClicked);
    connect(deleteInputField, &QLineEdit::returnPressed, this, &TreeDeletion::onDeleteClicked);
}

void TreeDeletion::onBackClicked()
{
    emit backToOperations();
}

void TreeDeletion::onInsertClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    QString text = insertInputField->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a value.");
        return;
    }

    bool ok;
    int value = text.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
        return;
    }

    insertNode(value);
    insertInputField->clear();
    insertInputField->setFocus();
    update();
}

void TreeDeletion::onDeleteClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    if (!root) {
        QMessageBox::warning(this, "Empty Tree", "Tree is empty. Nothing to delete.");
        return;
    }

    QString text = deleteInputField->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a value to delete.");
        return;
    }

    bool ok;
    int value = text.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
        return;
    }

    animateDeletion(value);
    deleteInputField->clear();
}

void TreeDeletion::onClearClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    clearTree(root);
    root = nullptr;
    statusLabel->setText("Tree cleared! Start by inserting values.");
    update();
}

void TreeDeletion::insertNode(int value)
{
    TreeNodeDel *newNode = new TreeNodeDel(value);

    if (!root) {
        root = newNode;
        statusLabel->setText(QString("Inserted %1 as root node").arg(value));
        return;
    }

    TreeNodeDel *current = root;
    TreeNodeDel *parent = nullptr;

    while (current) {
        parent = current;
        if (value < current->value) {
            current = current->left;
        } else if (value > current->value) {
            current = current->right;
        } else {
            delete newNode;
            statusLabel->setText(QString("Value %1 already exists!").arg(value));
            return;
        }
    }

    if (value < parent->value) {
        parent->left = newNode;
        statusLabel->setText(QString("Inserted %1 as left child of %2").arg(value).arg(parent->value));
    } else {
        parent->right = newNode;
        statusLabel->setText(QString("Inserted %1 as right child of %2").arg(value).arg(parent->value));
    }
}

void TreeDeletion::animateDeletion(int value)
{
    // Check if node exists
    TreeNodeDel *nodeToDelete = findNode(root, value);
    if (!nodeToDelete) {
        QMessageBox::warning(this, "Not Found",
                             QString("Value %1 not found in tree!").arg(value));
        return;
    }

    isAnimating = true;
    insertButton->setEnabled(false);
    deleteButton->setEnabled(false);
    clearButton->setEnabled(false);

    // Build search path
    searchPath.clear();
    TreeNodeDel *current = root;
    while (current) {
        searchPath.append(current);
        if (value < current->value) {
            current = current->left;
        } else if (value > current->value) {
            current = current->right;
        } else {
            break;
        }
    }

    currentSearchStep = 0;
    animateSearch(value, 0);
}

void TreeDeletion::animateSearch(int value, int step)
{
    if (step >= searchPath.size()) {
        // Found the node, now perform deletion
        resetHighlights(root);
        TreeNodeDel *nodeToDelete = findNode(root, value);
        if (nodeToDelete) {
            nodeToDelete->isToDelete = true;
            int children = countChildren(nodeToDelete);

            if (children == 0) {
                statusLabel->setText(QString("Deleting leaf node %1 (Case 1: No children)").arg(value));
            } else if (children == 1) {
                statusLabel->setText(QString("Deleting node %1 (Case 2: One child)").arg(value));
            } else {
                TreeNodeDel *successor = findMin(nodeToDelete->right);
                successor->isReplacement = true;
                statusLabel->setText(QString("Deleting node %1 (Case 3: Two children) - Replacing with %2").arg(value).arg(successor->value));
            }
        }
        update();

        QTimer::singleShot(1500, this, [this, value]() {
            deleteNode(value);
            resetHighlights(root);
            isAnimating = false;
            insertButton->setEnabled(true);
            deleteButton->setEnabled(true);
            clearButton->setEnabled(true);
            update();
        });
        return;
    }

    resetHighlights(root);
    searchPath[step]->isHighlighted = true;

    if (step == 0) {
        statusLabel->setText(QString("Searching for %1 - Checking root %2").arg(value).arg(searchPath[step]->value));
    } else {
        statusLabel->setText(QString("Searching for %1 - Checking node %2").arg(value).arg(searchPath[step]->value));
    }

    update();

    QTimer::singleShot(600, this, [this, value, step]() {
        animateSearch(value, step + 1);
    });
}

void TreeDeletion::deleteNode(int value)
{
    bool found = false;
    root = deleteNodeHelper(root, value, found);

    if (found) {
        statusLabel->setText(QString("Successfully deleted %1").arg(value));
    } else {
        statusLabel->setText(QString("Value %1 not found").arg(value));
    }
}

TreeNodeDel* TreeDeletion::deleteNodeHelper(TreeNodeDel* node, int value, bool &found)
{
    if (!node) return nullptr;

    if (value < node->value) {
        node->left = deleteNodeHelper(node->left, value, found);
    } else if (value > node->value) {
        node->right = deleteNodeHelper(node->right, value, found);
    } else {
        // Node found
        found = true;

        // Case 1: No children (leaf node)
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }

        // Case 2: One child
        if (!node->left) {
            TreeNodeDel *temp = node->right;
            delete node;
            return temp;
        }
        if (!node->right) {
            TreeNodeDel *temp = node->left;
            delete node;
            return temp;
        }

        // Case 3: Two children
        // Find inorder successor (smallest in right subtree)
        TreeNodeDel *successor = findMin(node->right);
        node->value = successor->value;
        node->right = deleteNodeHelper(node->right, successor->value, found);
    }

    return node;
}

TreeNodeDel* TreeDeletion::findMin(TreeNodeDel* node)
{
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

TreeNodeDel* TreeDeletion::findNode(TreeNodeDel* node, int value)
{
    if (!node) return nullptr;
    if (node->value == value) return node;
    if (value < node->value) return findNode(node->left, value);
    return findNode(node->right, value);
}

int TreeDeletion::countChildren(TreeNodeDel* node)
{
    if (!node) return 0;
    int count = 0;
    if (node->left) count++;
    if (node->right) count++;
    return count;
}

void TreeDeletion::calculateNodePositions(TreeNodeDel *node, int x, int y, int horizontalSpacing)
{
    if (!node) return;

    node->x = x;
    node->y = y;

    int nextSpacing = horizontalSpacing / 2;

    if (node->left) {
        calculateNodePositions(node->left, x - horizontalSpacing, y + LEVEL_HEIGHT, nextSpacing);
    }
    if (node->right) {
        calculateNodePositions(node->right, x + horizontalSpacing, y + LEVEL_HEIGHT, nextSpacing);
    }
}

void TreeDeletion::resetHighlights(TreeNodeDel *node)
{
    if (!node) return;
    node->isHighlighted = false;
    node->isToDelete = false;
    node->isReplacement = false;
    resetHighlights(node->left);
    resetHighlights(node->right);
}

void TreeDeletion::clearTree(TreeNodeDel *node)
{
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

void TreeDeletion::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));
    gradient.setColorAt(1.0, QColor(237, 228, 255));
    painter.fillRect(rect(), gradient);

    // Draw white canvas area for tree
    int canvasY = 320;
    int canvasHeight = height() - canvasY - 30;
    QRect canvasRect(30, canvasY, width() - 60, canvasHeight);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(canvasRect, 16, 16);

    // Draw tree if exists
    if (root) {
        int canvasWidth = canvasRect.width();
        int canvasCenterX = canvasRect.x() + canvasWidth / 2;
        int treeStartY = canvasRect.y() + 50;

        calculateNodePositions(root, canvasCenterX, treeStartY, canvasWidth / 4);
        drawTree(painter, root);
    }
}

void TreeDeletion::drawTree(QPainter &painter, TreeNodeDel *node)
{
    if (!node) return;

    // Draw edges first
    if (node->left) {
        drawEdge(painter, node->x, node->y, node->left->x, node->left->y);
        drawTree(painter, node->left);
    }
    if (node->right) {
        drawEdge(painter, node->x, node->y, node->right->x, node->right->y);
        drawTree(painter, node->right);
    }

    // Draw node on top
    drawNode(painter, node);
}

void TreeDeletion::drawNode(QPainter &painter, TreeNodeDel *node)
{
    if (!node) return;

    // Node circle with different colors based on state
    if (node->isToDelete) {
        painter.setPen(QPen(QColor(220, 53, 69), 4));
        painter.setBrush(QColor(255, 100, 120));
    } else if (node->isReplacement) {
        painter.setPen(QPen(QColor(40, 167, 69), 4));
        painter.setBrush(QColor(144, 238, 144));
    } else if (node->isHighlighted) {
        painter.setPen(QPen(QColor(255, 193, 7), 4));
        painter.setBrush(QColor(255, 220, 120));
    } else {
        painter.setPen(QPen(QColor(123, 79, 255), 3));
        painter.setBrush(QColor(200, 180, 255));
    }

    painter.drawEllipse(QPoint(node->x, node->y), NODE_RADIUS, NODE_RADIUS);

    // Node value
    painter.setPen(Qt::black);
    QFont font("Segoe UI", 14, QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRect(node->x - NODE_RADIUS, node->y - NODE_RADIUS,
                           NODE_RADIUS * 2, NODE_RADIUS * 2),
                     Qt::AlignCenter, QString::number(node->value));
}

void TreeDeletion::drawEdge(QPainter &painter, int x1, int y1, int x2, int y2)
{
    painter.setPen(QPen(QColor(123, 79, 255), 2));
    painter.drawLine(x1, y1 + NODE_RADIUS, x2, y2 - NODE_RADIUS);
}
