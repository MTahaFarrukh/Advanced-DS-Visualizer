#pragma once

#include <QString>
#include <QVector>
#include <QHashFunctions>
#include <forward_list>
#include <optional>
#include <vector>

// Open-chaining HashMap specialized for QString keys and values.
// Instrumented with a human-readable step trace for visualization.
class HashMap {
public:
    explicit HashMap(int initialBucketCount = 16, float maxLoadFactor = 0.75f);

    // Inserts a new key/value. Returns true if a new element was inserted,
    // false if an existing key was updated (no size change).
    bool insert(const QString &key, const QString &value);

    // Upsert variant: always assigns value (inserts if missing, updates if present).
    void put(const QString &key, const QString &value);

    // Looks up a key and returns the value if found.
    // This method records a detailed step trace for UI display.
    std::optional<QString> get(const QString &key);

    // Erases a key if present. Returns true if something was removed.
    bool erase(const QString &key);

    bool contains(const QString &key);

    void clear();

    int size() const;
    int bucketCount() const;
    float loadFactor() const;

    void rehash(int newBucketCount);
    void reserve(int expectedElements);

    // Visualization helpers
    const QVector<QString> &lastSteps() const;
    void clearSteps();
    QVector<int> bucketSizes() const;

private:
    struct Node {
        QString key;
        QString value;
    };

    std::vector<std::forward_list<Node>> buckets_;
    int numElements_ = 0;
    float maxLoadFactor_ = 0.75f;
    QVector<QString> lastSteps_;

    inline int indexFor(const QString &key, int bucketCount) const {
        // qHash returns a 32-bit unsigned; cast to size_t for modulo math
        const size_t hash = static_cast<size_t>(qHash(key));
        return static_cast<int>(hash % static_cast<size_t>(bucketCount));
    }

    void addStep(const QString &text);
    bool emplaceOrAssign(const QString &key, const QString &value, bool assignIfExists);
    void maybeGrow();
};


