#include "hashmap.h"

#include <algorithm>

HashMap::HashMap(int initialBucketCount, float maxLoadFactor)
    : buckets_(static_cast<size_t>(std::max(1, initialBucketCount))),
      numElements_(0),
      maxLoadFactor_(maxLoadFactor) {
    lastSteps_.clear();
}

void HashMap::addStep(const QString &text) {
    lastSteps_.push_back(text);
}

void HashMap::clearSteps() {
    lastSteps_.clear();
}

const QVector<QString> &HashMap::lastSteps() const {
    return lastSteps_;
}

int HashMap::size() const {
    return numElements_;
}

int HashMap::bucketCount() const {
    return static_cast<int>(buckets_.size());
}

float HashMap::loadFactor() const {
    if (buckets_.empty()) return 0.0f;
    return static_cast<float>(numElements_) / static_cast<float>(buckets_.size());
}

void HashMap::maybeGrow() {
    const float projected = (static_cast<float>(numElements_) + 1.0f)
        / static_cast<float>(buckets_.empty() ? 1 : buckets_.size());
    if (projected > maxLoadFactor_) {
        const int newCount = std::max(2, bucketCount() * 2);
        addStep(QStringLiteral("Load factor %.2f exceeds %.2f → rehash to %1 buckets")
                    .arg(newCount)
                    .arg(loadFactor(), 0, 'f', 2)
                    .arg(maxLoadFactor_, 0, 'f', 2));
        rehash(newCount);
    }
}

bool HashMap::emplaceOrAssign(const QString &key, const QString &value, bool assignIfExists) {
    const int bucketCountNow = bucketCount();
    const size_t hash = static_cast<size_t>(qHash(key));
    const int index = static_cast<int>(hash % static_cast<size_t>(bucketCountNow));

    addStep(QStringLiteral("Compute hash(%1) = %2").arg(key).arg(static_cast<qulonglong>(hash)));
    addStep(QStringLiteral("Index = hash %% %1 = %2").arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    auto &chain = buckets_[static_cast<size_t>(index)];
    for (auto &node : chain) {
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(node.key, key, node.key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (node.key == key) {
            if (assignIfExists) {
                addStep(QStringLiteral("Key exists → update value: %1 → %2").arg(node.value, value));
                node.value = value;
            } else {
                addStep(QStringLiteral("Key exists → no insert (duplicate)"));
            }
            return false; // not a new insertion
        }
        addStep(QStringLiteral("Traverse next in chain"));
    }

    addStep(QStringLiteral("Append new node to bucket %1").arg(index));
    chain.push_front(Node{key, value});
    ++numElements_;
    addStep(QStringLiteral("New size = %1, load factor = %2")
                .arg(numElements_)
                .arg(loadFactor(), 0, 'f', 2));
    return true;
}

bool HashMap::insert(const QString &key, const QString &value) {
    clearSteps();
    maybeGrow();
    return emplaceOrAssign(key, value, /*assignIfExists=*/false);
}

void HashMap::put(const QString &key, const QString &value) {
    clearSteps();
    maybeGrow();
    (void)emplaceOrAssign(key, value, /*assignIfExists=*/true);
}

std::optional<QString> HashMap::get(const QString &key) {
    clearSteps();
    if (buckets_.empty()) {
        addStep(QStringLiteral("Table is empty → not found"));
        return std::nullopt;
    }

    const int bucketCountNow = bucketCount();
    const size_t hash = static_cast<size_t>(qHash(key));
    const int index = static_cast<int>(hash % static_cast<size_t>(bucketCountNow));

    addStep(QStringLiteral("Compute hash(%1) = %2").arg(key).arg(static_cast<qulonglong>(hash)));
    addStep(QStringLiteral("Index = hash %% %1 = %2").arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    const auto &chain = buckets_[static_cast<size_t>(index)];
    for (const auto &node : chain) {
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(node.key, key, node.key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (node.key == key) {
            addStep(QStringLiteral("Found → return value %1").arg(node.value));
            return node.value;
        }
        addStep(QStringLiteral("Traverse next in chain"));
    }
    addStep(QStringLiteral("Reached end of chain → not found"));
    return std::nullopt;
}

bool HashMap::erase(const QString &key) {
    clearSteps();
    if (buckets_.empty()) {
        addStep(QStringLiteral("Table is empty → nothing to erase"));
        return false;
    }

    const int bucketCountNow = bucketCount();
    const size_t hash = static_cast<size_t>(qHash(key));
    const int index = static_cast<int>(hash % static_cast<size_t>(bucketCountNow));

    addStep(QStringLiteral("Compute hash(%1) = %2").arg(key).arg(static_cast<qulonglong>(hash)));
    addStep(QStringLiteral("Index = hash %% %1 = %2").arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    auto &chain = buckets_[static_cast<size_t>(index)];
    auto before = chain.before_begin();
    for (auto it = chain.begin(); it != chain.end(); ++it) {
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(it->key, key, it->key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (it->key == key) {
            chain.erase_after(before);
            --numElements_;
            addStep(QStringLiteral("Erased node. New size = %1, load factor = %2")
                        .arg(numElements_)
                        .arg(loadFactor(), 0, 'f', 2));
            return true;
        }
        ++before;
        addStep(QStringLiteral("Traverse next in chain"));
    }
    addStep(QStringLiteral("Reached end of chain → key not found"));
    return false;
}

bool HashMap::contains(const QString &key) {
    return get(key).has_value();
}

void HashMap::clear() {
    clearSteps();
    for (auto &chain : buckets_) {
        chain.clear();
    }
    numElements_ = 0;
    addStep(QStringLiteral("Cleared all buckets"));
}

void HashMap::rehash(int newBucketCount) {
    if (newBucketCount < 1) newBucketCount = 1;
    QVector<QString> rehashSteps;
    rehashSteps.push_back(QStringLiteral("Rehashing to %1 buckets").arg(newBucketCount));

    std::vector<std::forward_list<Node>> newBuckets(static_cast<size_t>(newBucketCount));
    for (auto &chain : buckets_) {
        for (auto &node : chain) {
            const int newIndex = indexFor(node.key, newBucketCount);
            rehashSteps.push_back(QStringLiteral("Move (%1,%2) → bucket %3")
                                      .arg(node.key, node.value)
                                      .arg(newIndex));
            newBuckets[static_cast<size_t>(newIndex)].push_front(Node{std::move(node.key), std::move(node.value)});
        }
    }
    buckets_.swap(newBuckets);
    // Append rehash steps to the live steps log.
    for (const auto &s : rehashSteps) lastSteps_.push_back(s);
}

void HashMap::reserve(int expectedElements) {
    if (expectedElements <= 0) return;
    const float desiredLoad = 0.6f; // target below max for headroom
    const int requiredBuckets = std::max(1, static_cast<int>(expectedElements / desiredLoad));
    if (requiredBuckets > bucketCount()) {
        addStep(QStringLiteral("Reserve(%1) → rehash to %2 buckets")
                    .arg(expectedElements)
                    .arg(requiredBuckets));
        rehash(requiredBuckets);
    }
}

QVector<int> HashMap::bucketSizes() const {
    QVector<int> sizes;
    sizes.reserve(static_cast<int>(buckets_.size()));
    for (const auto &chain : buckets_) {
        int count = 0;
        for (const auto &n : chain) {
            (void)n; // suppress unused variable warning
            ++count;
        }
        sizes.push_back(count);
    }
    return sizes;
}


