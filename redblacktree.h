#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QTextEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollArea>

enum Color { RED, BLACK };

struct RBNode {
    int value;
    Color color;
    RBNode *left;
    RBNode *right;
    RBNode *parent;
    int x, y;
    bool isHighlighted;
    bool isRotating;

    RBNode(int val) : value(val), color(RED), left(nullptr), right(nullptr),
        parent(nullptr), x(0), y(0), isHighlighted(false),
        isRotating(false) {}
};

struct HistoryEntry {
    QString operation;
    int value;
    QString description;
    QString timestamp;
};

class RedBlackTree : public QWidget
{
    Q_OBJECT

public:
    explicit RedBlackTree(QWidget *parent = nullptr);
    ~RedBlackTree();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBackClicked();
    void onInsertClicked();
    void onDeleteClicked();
    void onSearchClicked();
    void onClearClicked();
    void onViewAlgorithmClicked();
    void onAlgorithmBackClicked();
    void showInsertionAlgorithm();
    void showDeletionAlgorithm();
    void showSearchAlgorithm();

private:
    void setupUI();
    void setupAlgorithmView();

    // RB Tree operations
    void insertNode(int value);
    void deleteNode(int value);
    void searchNode(int value);
    RBNode* BSTInsert(RBNode* root, RBNode* node);
    void fixInsert(RBNode* node);
    void rotateLeft(RBNode* node);
    void rotateRight(RBNode* node);
    void rotateLeftSync(RBNode* node);  // Synchronous rotation for fixInsert
    void rotateRightSync(RBNode* node); // Synchronous rotation for fixInsert
    RBNode* findNode(RBNode* node, int value);
    void fixDelete(RBNode* node);
    RBNode* findMin(RBNode* node);
    void transplant(RBNode* u, RBNode* v);
    RBNode* deleteNodeHelper(RBNode* node, int value);

    // Animation
    void animateRotation(RBNode* node, const QString& direction);
    void highlightNode(RBNode* node, int delay);

    // Drawing
    void calculateNodePositions(RBNode *node, int x, int y, int horizontalSpacing);
    void drawTree(QPainter &painter, RBNode *node);
    void drawNode(QPainter &painter, RBNode *node);
    void drawEdge(QPainter &painter, int x1, int y1, int x2, int y2, Color color);
    void clearTree(RBNode *node);
    void resetHighlights(RBNode *node);

    // History
    void addHistory(const QString &operation, int value, const QString &description);
    QString getCurrentTime();

    // UI Components - Main View
    QStackedWidget *mainStack;
    QWidget *treeViewWidget;
    QWidget *algorithmViewWidget;

    QPushButton *backButton;
    QPushButton *insertButton;
    QPushButton *deleteButton;
    QPushButton *searchButton;
    QPushButton *clearButton;
    QPushButton *viewAlgorithmButton;

    QLineEdit *inputField;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QListWidget *historyList;

    // Algorithm View Components
    QPushButton *algorithmBackButton;
    QPushButton *insertAlgoButton;
    QPushButton *deleteAlgoButton;
    QPushButton *searchAlgoButton;
    QTextEdit *algorithmDisplay;
    QLabel *algorithmTitleLabel;

    // Tree data
    RBNode *root;
    RBNode *NIL;  // Sentinel node

    // History
    QVector<HistoryEntry> history;

    // Animation
    bool isAnimating;
    QTimer *animationTimer;

    // Drawing constants
    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
};

#endif // REDBLACKTREE_H
