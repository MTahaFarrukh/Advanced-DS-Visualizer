#include "theorypage.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QFontDatabase>

TheoryPage::TheoryPage(const QString &dataStructureName, QWidget *parent)
    : QWidget(parent)
    , dsName(dataStructureName)
{
    setupUI();
    loadTheoryContent();
    setMinimumSize(900, 750);
}

TheoryPage::~TheoryPage()
{
}

void TheoryPage::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 35, 40, 35);
    mainLayout->setSpacing(20);

    // Header section
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Back button
    backButton = new QPushButton("← Back to Menu", this);
    backButton->setFixedSize(140, 38);
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

    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // Title
    titleLabel = new QLabel(dsName, this);
    QFont titleFont("Segoe UI", 36, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(5);

    // Scroll area for content
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: none;
            background: transparent;
        }
        QScrollBar:vertical {
            background: rgba(123, 79, 255, 0.1);
            width: 10px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: rgba(123, 79, 255, 0.5);
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(123, 79, 255, 0.7);
        }
    )");

    contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(10, 10, 10, 10);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // Try it yourself button
    tryButton = new QPushButton("Try It Yourself →", this);
    tryButton->setFixedSize(220, 55);
    tryButton->setContentsMargins(50,200,50,100);
    tryButton->setCursor(Qt::PointingHandCursor);

    QFont tryFont("Segoe UI", 15, QFont::Bold);
    tryButton->setFont(tryFont);
    tryButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 27px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a32cc, stop:1 #7a4ccc);
        }
    )");

    mainLayout->addWidget(tryButton, 0, Qt::AlignCenter);

    setLayout(mainLayout);

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &TheoryPage::backToMenu);
    connect(tryButton, &QPushButton::clicked, this, &TheoryPage::tryItYourself);
}

void TheoryPage::loadTheoryContent()
{
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    if (!contentLayout) return;

    // Clear existing content
    QLayoutItem *item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add theory cards
    contentLayout->addWidget(createInfoCard("📖 Definition", getDefinition(), "📖"));
    contentLayout->addWidget(createInfoCard("⏱️ Time Complexity", getTimeComplexity(), "⏱️"));
    contentLayout->addWidget(createInfoCard("💡 Applications", getApplications(), "💡"));
    contentLayout->addWidget(createInfoCard("✅ Advantages", getAdvantages(), "✅"));
    contentLayout->addWidget(createInfoCard("⚠️ Disadvantages", getDisadvantages(), "⚠️"));

    contentLayout->addStretch();
}

QWidget* TheoryPage::createInfoCard(const QString &title, const QString &content, const QString &icon)
{
    QFrame *card = new QFrame();
    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            padding: 20px;
        }
    )");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(12);

    // Title with icon
    QLabel *titleLabel = new QLabel(title);
    QFont titleFont("Segoe UI", 16, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2d1b69; background: transparent;");
    cardLayout->addWidget(titleLabel);

    // Content
    QLabel *contentLabel = new QLabel(content);
    QFont contentFont("Segoe UI", 12);
    contentLabel->setFont(contentFont);
    contentLabel->setStyleSheet("color: #4a4a4a; background: transparent; line-height: 1.6;");
    contentLabel->setWordWrap(true);
    contentLabel->setTextFormat(Qt::RichText);
    cardLayout->addWidget(contentLabel);

    card->setLayout(cardLayout);
    return card;
}

QString TheoryPage::getDefinition()
{
    if (dsName == "Binary Tree") {
        return "A <b>Binary Tree</b> is a hierarchical data structure in which each node has at most two children, "
               "referred to as the <b>left child</b> and <b>right child</b>. It starts with a root node and "
               "branches out into a tree-like structure. Binary trees are fundamental in computer science and "
               "form the basis for more complex data structures like Binary Search Trees, AVL Trees, and Heaps.";
    } else if (dsName == "Red-Black Tree") {
        return "A <b>Red-Black Tree</b> is a self-balancing Binary Search Tree where each node has an extra bit "
               "for denoting the color (red or black). These color bits ensure that the tree remains approximately "
               "balanced during insertions and deletions, guaranteeing O(log n) time complexity for operations.";
    } else if (dsName == "Graph") {
        return "A <b>Graph</b> is a non-linear data structure consisting of vertices (nodes) and edges that connect "
               "pairs of vertices. Graphs can be directed or undirected, weighted or unweighted, and are used to "
               "represent networks, relationships, and connections between entities.";
    } else if (dsName == "Hash Table") {
        return "A <b>Hash Table</b> (Hash Map) is a data structure that implements an associative array, mapping "
               "keys to values. It uses a hash function to compute an index into an array of buckets, from which "
               "the desired value can be found, providing average-case O(1) time complexity for search operations.";
    }
    return "Information not available.";
}

QString TheoryPage::getTimeComplexity()
{
    if (dsName == "Binary Tree") {
        return "<b>Search:</b> O(n) in worst case (unbalanced), O(log n) for balanced trees<br>"
               "<b>Insertion:</b> O(n) in worst case, O(log n) for balanced trees<br>"
               "<b>Deletion:</b> O(n) in worst case, O(log n) for balanced trees<br>"
               "<b>Space Complexity:</b> O(n)";
    } else if (dsName == "Red-Black Tree") {
        return "<b>Search:</b> O(log n)<br>"
               "<b>Insertion:</b> O(log n)<br>"
               "<b>Deletion:</b> O(log n)<br>"
               "<b>Space Complexity:</b> O(n)";
    } else if (dsName == "Graph") {
        return "<b>Search (BFS/DFS):</b> O(V + E) where V=vertices, E=edges<br>"
               "<b>Insertion:</b> O(1) for adding vertex/edge<br>"
               "<b>Deletion:</b> O(V + E) in worst case<br>"
               "<b>Space Complexity:</b> O(V + E)";
    } else if (dsName == "Hash Table") {
        return "<b>Search:</b> O(1) average case, O(n) worst case<br>"
               "<b>Insertion:</b> O(1) average case, O(n) worst case<br>"
               "<b>Deletion:</b> O(1) average case, O(n) worst case<br>"
               "<b>Space Complexity:</b> O(n)";
    }
    return "Information not available.";
}

QString TheoryPage::getApplications()
{
    if (dsName == "Binary Tree") {
        return "• <b>Expression Trees:</b> Used in compilers for parsing expressions<br>"
               "• <b>Binary Search Trees:</b> Efficient searching and sorting<br>"
               "• <b>Huffman Coding Trees:</b> Data compression algorithms<br>"
               "• <b>File System:</b> Directory structure representation<br>"
               "• <b>Database Indexing:</b> B-trees for efficient data retrieval";
    } else if (dsName == "Red-Black Tree") {
        return "• <b>Associative Arrays:</b> Implementation of maps and sets in C++ STL<br>"
               "• <b>Java TreeMap:</b> Sorted map implementation<br>"
               "• <b>Linux Kernel:</b> Completely Fair Scheduler (CFS)<br>"
               "• <b>Memory Management:</b> Virtual memory management in operating systems";
    } else if (dsName == "Graph") {
        return "• <b>Social Networks:</b> Representing connections between users<br>"
               "• <b>Maps & Navigation:</b> GPS and route finding (Dijkstra's algorithm)<br>"
               "• <b>Computer Networks:</b> Network topology and routing protocols<br>"
               "• <b>Web Crawlers:</b> Link structure of websites<br>"
               "• <b>Recommendation Systems:</b> Product and content recommendations";
    } else if (dsName == "Hash Table") {
        return "• <b>Databases:</b> Fast data retrieval and indexing<br>"
               "• <b>Caching:</b> Quick access to frequently used data<br>"
               "• <b>Symbol Tables:</b> Compiler design and interpreters<br>"
               "• <b>Password Verification:</b> Storing hashed passwords securely<br>"
               "• <b>Dictionaries:</b> Implementing key-value storage systems";
    }
    return "Information not available.";
}

QString TheoryPage::getAdvantages()
{
    if (dsName == "Binary Search Tree") {
        return "• Simple and intuitive hierarchical structure<br>"
               "• Forms the basis for more advanced tree structures<br>"
               "• Efficient searching in balanced binary search trees<br>"
               "• Natural representation of hierarchical data<br>"
               "• Supports efficient in-order, pre-order, and post-order traversals";
    } else if (dsName == "Red-Black Tree") {
        return "• Guaranteed O(log n) time complexity for operations<br>"
               "• Self-balancing ensures consistent performance<br>"
               "• Better than AVL trees for insertion-heavy workloads<br>"
               "• Widely used in production systems<br>"
               "• Requires at most 2 rotations for insertion";
    } else if (dsName == "Graph") {
        return "• Can represent complex relationships and networks<br>"
               "• Flexible structure accommodates various real-world scenarios<br>"
               "• Rich algorithms available (shortest path, MST, etc.)<br>"
               "• Both directed and undirected relationships supported<br>"
               "• Can model cyclic and acyclic dependencies";
    } else if (dsName == "Hash Table") {
        return "• Extremely fast average-case O(1) lookup, insertion, and deletion<br>"
               "• Efficient memory usage with proper load factor<br>"
               "• Simple to implement and use<br>"
               "• Ideal for key-value pair storage<br>"
               "• Widely supported in programming languages";
    }
    return "Information not available.";
}

QString TheoryPage::getDisadvantages()
{
    if (dsName == "Binary Tree") {
        return "• Can become unbalanced, leading to O(n) operations<br>"
               "• No guaranteed performance without balancing<br>"
               "• Requires extra memory for pointers<br>"
               "• Complex deletion operation (especially with two children)<br>"
               "• Not efficient for unsorted data";
    } else if (dsName == "Red-Black Tree") {
        return "• More complex implementation than basic binary trees<br>"
               "• Requires extra storage for color information<br>"
               "• More rotations needed than some other balanced trees<br>"
               "• Harder to understand and debug<br>"
               "• Slightly slower than AVL trees for search-heavy workloads";
    } else if (dsName == "Graph") {
        return "• Can consume significant memory for dense graphs<br>"
               "• Complex algorithms may be difficult to implement<br>"
               "• No fixed time complexity for many operations<br>"
               "• Cycle detection can be computationally expensive<br>"
               "• Requires careful handling of disconnected components";
    } else if (dsName == "Hash Table") {
        return "• Worst-case O(n) time complexity due to collisions<br>"
               "• No ordering of elements maintained<br>"
               "• Requires good hash function to avoid clustering<br>"
               "• Resizing can be expensive<br>"
               "• Vulnerable to hash collision attacks (DoS)";
    }
    return "Information not available.";
}

void TheoryPage::paintEvent(QPaintEvent *event)
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
