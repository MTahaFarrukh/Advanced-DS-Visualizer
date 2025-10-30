#include "hashmap.h"

#include <algorithm>
#include <cmath>

HashMap::HashMap(int initialBucketCount, float maxLoadFactor)
    : buckets_(static_cast<size_t>(std::max(1, initialBucketCount))),
      numElements_(0),
      maxLoadFactor_(maxLoadFactor) {
    stepHistory_.clear();
}

QString HashMap::dataTypeToString(DataType type) {
    switch (type) {
        case STRING: return "String";
        case INTEGER: return "Integer";
        case DOUBLE: return "Double";
        case FLOAT: return "Float";
        default: return "Unknown";
    }
}

QString HashMap::variantToDisplayString(const QVariant &var) {
    if (var.type() == QVariant::String) {
        return var.toString();
    } else if (var.type() == QVariant::Int) {
        return QString::number(var.toInt());
    } else if (var.type() == QVariant::Double) {
        return QString::number(var.toDouble(), 'f', 2);
        } else if (var.canConvert<float>()) {
        return QString::number(var.toFloat(), 'f', 2);
    }
    return var.toString();
}

int HashMap::indexFor(const QVariant &key, int bucketCount) const {
    size_t hash;
    
    // Simple hash functions for educational purposes
    switch (key.type()) {
        case QVariant::String: {
            // For strings, use Qt's hash function
            hash = qHash(key.toString());
            break;
        }
        case QVariant::Int: {
            // For integers, hash = key itself (simple modulo will work)
            int intKey = key.toInt();
            hash = static_cast<size_t>(intKey >= 0 ? intKey : -intKey); // Use absolute value for negative numbers
            break;
        }
        case QVariant::Double: {
            // For double/float, convert to int for simplicity
            double doubleKey = key.toDouble();
            hash = static_cast<size_t>(std::abs(static_cast<int>(doubleKey)));
            break;
        }
        default:
            hash = qHash(key.toString());
    }
    
    // Standard hash function: hash % n
    return static_cast<int>(hash % static_cast<size_t>(bucketCount));
}

bool HashMap::validateType(const QVariant &value, DataType expectedType) const {
    switch (expectedType) {
        case STRING:
            return value.canConvert<QString>();
        case INTEGER:
            return value.canConvert<int>();
        case DOUBLE:
            return value.canConvert<double>();
        case FLOAT:
            return value.canConvert<float>();
        default:
            return false;
    }
}

void HashMap::addStep(const QString &text) {
    stepHistory_.append(text);
}

void HashMap::addStepToHistory(const QString &step) {
    stepHistory_.append(step);
}

void HashMap::clearSteps() {
    // Don't clear history, just mark a separator
    stepHistory_.append("--- Operation Complete ---");
}

const QVector<QString> &HashMap::lastSteps() const {
    return stepHistory_;
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

bool HashMap::emplaceOrAssign(const QVariant &key, const QVariant &value, bool assignIfExists) {
    // Validate types
    if (!validateType(key, keyType_) || !validateType(value, valueType_)) {
        addStep(QStringLiteral("Type validation failed"));
        return false;
    }

    const int bucketCountNow = bucketCount();
    
    QString keyStr = variantToDisplayString(key);
    QString valueStr = variantToDisplayString(value);
    
    // Use our custom indexFor method which shows the simple hash
    const int index = indexFor(key, bucketCountNow);
    
    addStep(QStringLiteral("Index = %1 %% %2 = %3").arg(keyStr).arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    auto &chain = buckets_[static_cast<size_t>(index)];
    for (auto &node : chain) {
        QString nodeKeyStr = variantToDisplayString(node.key);
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(nodeKeyStr, keyStr, node.key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (node.key == key) {
            if (assignIfExists) {
                QString oldValueStr = variantToDisplayString(node.value);
                addStep(QStringLiteral("Key exists → update value: %1 → %2").arg(oldValueStr, valueStr));
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

bool HashMap::insert(const QVariant &key, const QVariant &value) {
    addStep(QStringLiteral("=== INSERT OPERATION ==="));
    maybeGrow();
    bool result = emplaceOrAssign(key, value, /*assignIfExists=*/false);
    clearSteps();
    return result;
}

void HashMap::put(const QVariant &key, const QVariant &value) {
    addStep(QStringLiteral("=== PUT OPERATION ==="));
    maybeGrow();
    (void)emplaceOrAssign(key, value, /*assignIfExists=*/true);
    clearSteps();
}

std::optional<QVariant> HashMap::get(const QVariant &key) {
    addStep(QStringLiteral("=== SEARCH OPERATION ==="));
    if (buckets_.empty()) {
        addStep(QStringLiteral("Table is empty → not found"));
        clearSteps();
        return std::nullopt;
    }

    const int bucketCountNow = bucketCount();
    
    QString keyStr = variantToDisplayString(key);
    
    // Use our custom indexFor method
    const int index = indexFor(key, bucketCountNow);
    
    addStep(QStringLiteral("Index = %1 %% %2 = %3").arg(keyStr).arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    const auto &chain = buckets_[static_cast<size_t>(index)];
    for (const auto &node : chain) {
        QString nodeKeyStr = variantToDisplayString(node.key);
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(nodeKeyStr, keyStr, node.key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (node.key == key) {
            QString valueStr = variantToDisplayString(node.value);
            addStep(QStringLiteral("Found → return value %1").arg(valueStr));
            clearSteps();
            return node.value;
        }
        addStep(QStringLiteral("Traverse next in chain"));
    }
    addStep(QStringLiteral("Reached end of chain → not found"));
    clearSteps();
    return std::nullopt;
}

bool HashMap::erase(const QVariant &key) {
    addStep(QStringLiteral("=== DELETE OPERATION ==="));
    if (buckets_.empty()) {
        addStep(QStringLiteral("Table is empty → nothing to erase"));
        clearSteps();
        return false;
    }

    const int bucketCountNow = bucketCount();
    
    QString keyStr = variantToDisplayString(key);
    
    // Use our custom indexFor method
    const int index = indexFor(key, bucketCountNow);
    
    addStep(QStringLiteral("Index = %1 %% %2 = %3").arg(keyStr).arg(bucketCountNow).arg(index));
    addStep(QStringLiteral("Visit bucket %1").arg(index));

    auto &chain = buckets_[static_cast<size_t>(index)];
    auto before = chain.before_begin();
    for (auto it = chain.begin(); it != chain.end(); ++it) {
        QString nodeKeyStr = variantToDisplayString(it->key);
        addStep(QStringLiteral("Compare keys: %1 == %2 ? %3")
                    .arg(nodeKeyStr, keyStr, it->key == key ? QStringLiteral("Yes") : QStringLiteral("No")));
        if (it->key == key) {
            chain.erase_after(before);
            --numElements_;
            addStep(QStringLiteral("Erased node. New size = %1, load factor = %2")
                        .arg(numElements_)
                        .arg(loadFactor(), 0, 'f', 2));
            clearSteps();
            return true;
        }
        ++before;
        addStep(QStringLiteral("Traverse next in chain"));
    }
    addStep(QStringLiteral("Reached end of chain → key not found"));
    clearSteps();
    return false;
}

bool HashMap::contains(const QVariant &key) {
    return get(key).has_value();
}

std::optional<QVariant> HashMap::findByValue(const QVariant &value) {
    addStep("🔍 Searching for value...");
    QString valueStr = variantToDisplayString(value);
    addStep(QString("Looking for value: %1").arg(valueStr));
    
    // Search through all buckets
    for (size_t i = 0; i < buckets_.size(); ++i) {
        addStep(QString("Checking bucket %1...").arg(i));
        
        for (const auto &node : buckets_[i]) {
            QString currentValue = variantToDisplayString(node.value);
            
            // Compare values
            if (node.value == value) {
                QString keyStr = variantToDisplayString(node.key);
                addStep(QString("✅ Found! Value '%1' has Key '%2' in bucket %3")
                       .arg(valueStr, keyStr).arg(i));
                return node.key; // Return the key associated with this value
            }
        }
    }
    
    addStep(QString("❌ Value '%1' not found in any bucket").arg(valueStr));
    return std::nullopt;
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
            rehashSteps.append(QStringLiteral("Move (%1,%2) → bucket %3")
                                      .arg(variantToDisplayString(node.key), variantToDisplayString(node.value))
                                      .arg(newIndex));
            newBuckets[static_cast<size_t>(newIndex)].push_front(Node{std::move(node.key), std::move(node.value)});
        }
    }
    buckets_.swap(newBuckets);
    // Append rehash steps to the live steps log.
    for (const auto &s : rehashSteps) stepHistory_.append(s);
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

QVector<QVector<QPair<QVariant, QVariant>>> HashMap::getBucketContents() const {
    QVector<QVector<QPair<QVariant, QVariant>>> contents;
    contents.reserve(static_cast<int>(buckets_.size()));
    
    for (const auto &chain : buckets_) {
        QVector<QPair<QVariant, QVariant>> bucketItems;
        for (const auto &node : chain) {
            bucketItems.push_back(QPair<QVariant, QVariant>(node.key, node.value));
        }
        contents.push_back(bucketItems);
    }
    
    return contents;
}


