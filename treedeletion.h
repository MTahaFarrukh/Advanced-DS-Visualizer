#ifndef TREEDELETION_H
#define TREEDELETION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QVector>
#include <QGraphicsOpacityEffect>

// Tree Node structure
struct TreeNodeDel {
    int value;
    TreeNodeDel *left;
    TreeNodeDel *right;
    int x, y;  // Position for drawing
    bool isHighlighted;
    bool isToDelete;
    bool isReplacement;

    TreeNodeDel(int val) : value(val), left(nullptr), right(nullptr),
        x(0), y(0), isHighlighted(false), isToDelete(false),
        isReplacement(false) {}
};

class TreeDeletion : public QWidget
{
    Q_OBJECT

public:
    explicit TreeDeletion(QWidget *parent = nullptr);
    ~TreeDeletion();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBackClicked();
    void onDeleteClicked();
    void onInsertClicked();
    void onClearClicked();

private:
    void setupUI();
    void insertNode(int value);
    void deleteNode(int value);
    void animateDeletion(int value);
    TreeNodeDel* deleteNodeHelper(TreeNodeDel* node, int value, bool &found);
    TreeNodeDel* findMin(TreeNodeDel* node);
    void calculateNodePositions(TreeNodeDel *node, int x, int y, int horizontalSpacing);
    void drawTree(QPainter &painter, TreeNodeDel *node);
    void drawNode(QPainter &painter, TreeNodeDel *node);
    void drawEdge(QPainter &painter, int x1, int y1, int x2, int y2);
    void clearTree(TreeNodeDel *node);
    void resetHighlights(TreeNodeDel *node);
    void animateSearch(int value, int step);
    TreeNodeDel* findNode(TreeNodeDel* node, int value);
    int countChildren(TreeNodeDel* node);

    // UI Components
    QPushButton *backButton;
    QPushButton *deleteButton;
    QPushButton *insertButton;
    QPushButton *clearButton;
    QLineEdit *deleteInputField;
    QLineEdit *insertInputField;
    QLabel *titleLabel;
    QLabel *instructionLabel;
    QLabel *statusLabel;

    // Tree data
    TreeNodeDel *root;

    // Animation
    QTimer *animationTimer;
    QVector<TreeNodeDel*> searchPath;
    int currentSearchStep;
    bool isAnimating;

    // Drawing constants
    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
    const int CANVAS_TOP_MARGIN = 60;
};

#endif // TREEDELETION_H
