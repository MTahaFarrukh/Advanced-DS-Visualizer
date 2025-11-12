#ifndef GRAPHVISUALIZATION_H
#define GRAPHVISUALIZATION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QTimer>
#include <QSet>
#include <QHash>
#include <QListWidget>
#include <QVector>
#include <QDateTime>

struct GraphNode {
    int id;
    QPointF pos;
    bool highlighted;
    bool visited;
    GraphNode() : id(-1), pos(0, 0), highlighted(false), visited(false) {}
    GraphNode(int nid, const QPointF &p) : id(nid), pos(p), highlighted(false), visited(false) {}
};

struct GraphHistoryEntry {
    QString operation;
    int value1;
    int value2;
    QString description;
    QString timestamp;
};

class GraphVisualization : public QWidget
{
    Q_OBJECT

public:
    explicit GraphVisualization(QWidget *parent = nullptr);
    ~GraphVisualization();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onBackClicked();
    void onAddVertexClicked();
    void onAddEdgeClicked();
    void onRemoveVertexClicked();
    void onRemoveEdgeClicked();
    void onClearClicked();
    void onStartBFS();
    void onStartDFS();
    void onAnimationStep();

private:
    // UI setup
    void setupUI();
    void restyleCombos();
    void refreshCombos();
    void setControlsEnabled(bool enabled);

    // Graph logic
    void layoutNodes();
    void resetHighlights();
    void addVertexInternal();
    bool addEdgeInternal(int u, int v);
    bool removeVertexInternal(int u);
    bool removeEdgeInternal(int u, int v);

    // Drawing helpers
    void drawGraph(QPainter &painter);
    void drawEdge(QPainter &painter, const QPointF &a, const QPointF &b, bool highlighted) const;
    void drawNode(QPainter &painter, const GraphNode &node) const;

    // History/Logging
    void addHistory(const QString &operation, int value1, int value2, const QString &description);
    QString getCurrentTime();

    // Animation state
    enum class TraversalType { None, BFS, DFS };
    TraversalType traversalType;
    QList<int> traversalOrder;
    int traversalIndex;
    QSet<int> frontier;
    QTimer *animTimer;

    // UI
    QPushButton *backButton;
    QLabel *titleLabel;
    QLabel *instructionLabel;
    QLabel *statusLabel;

    QPushButton *addVertexButton;
    QLineEdit *vertexInput;
    QComboBox *edgeFromCombo;
    QComboBox *edgeToCombo;
    QPushButton *addEdgeButton;

    QComboBox *removeVertexCombo;
    QPushButton *removeVertexButton;
    QComboBox *removeEdgeFromCombo;
    QComboBox *removeEdgeToCombo;
    QPushButton *removeEdgeButton;

    QComboBox *startCombo;
    QPushButton *bfsButton;
    QPushButton *dfsButton;
    QPushButton *clearButton;

    // History panel
    QListWidget *historyList;

    // Data
    QVector<GraphNode> nodes;
    QHash<int, QSet<int>> adjacency; // undirected, unweighted
    int nextId;
    QVector<GraphHistoryEntry> history;

    // Drawing constants
    const int NODE_RADIUS = 24;
};

#endif // GRAPHVISUALIZATION_H




