#ifndef HASHMAPVISUALIZATION_H
#define HASHMAPVISUALIZATION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QScrollArea>
#include <QSplitter>
#include "hashmap.h"

class HashMapVisualization : public QWidget
{
    Q_OBJECT

public:
    explicit HashMapVisualization(QWidget *parent = nullptr);
    ~HashMapVisualization();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBackClicked();
    void onInsertClicked();
    void onSearchClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onRandomizeClicked();
    void updateVisualization();
    void updateStepTrace();

private:
    void setupUI();
    void setupVisualizationArea();
    void setupControlPanel();
    void setupStepTracePanel();
    void drawBuckets();
    void animateOperation(const QString &operation);
    void animateSearchResult(const QString &key, bool found);
    void showStats();

    // UI Components
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QWidget *rightPanel;
    
    // Left panel - visualization and controls
    QVBoxLayout *leftLayout;
    QPushButton *backButton;
    QLabel *titleLabel;
    QGraphicsView *visualizationView;
    QGraphicsScene *scene;
    
    // Control panel
    QWidget *controlPanel;
    QLineEdit *keyInput;
    QLineEdit *valueInput;
    QPushButton *insertButton;
    QPushButton *searchButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;
    QPushButton *randomizeButton;
    
    // Stats panel
    QLabel *sizeLabel;
    QLabel *bucketCountLabel;
    QLabel *loadFactorLabel;
    
    // Right panel - step trace
    QVBoxLayout *rightLayout;
    QLabel *stepsTitle;
    QListWidget *stepsList;
    
    // Data and visualization
    HashMap *hashMap;
    QVector<QGraphicsRectItem*> bucketRects;
    QVector<QGraphicsTextItem*> bucketTexts;
    QVector<QVector<QGraphicsTextItem*>> chainTexts;
    
    // Animation
    QTimer *animationTimer;
    QGraphicsRectItem *highlightRect;
    
    // Constants
    static const int BUCKET_WIDTH;
    static const int BUCKET_HEIGHT;
    static const int BUCKET_SPACING;
    static const int CHAIN_ITEM_HEIGHT;
    static const int MAX_VISIBLE_BUCKETS;
};

#endif // HASHMAPVISUALIZATION_H
