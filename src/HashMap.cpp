#include "HashMap.h"

// djb2 string hash by Dan Bernstein.
// Chosen for its simple integer-arithmetic implementation and good
// distribution over short ASCII keys (e.g. category names like "Food").
int HashMap::hash(const std::string& key) const {
    unsigned long h = 5381;
    for (char c : key) {
        h = ((h << 5) + h) + static_cast<unsigned char>(c); // h * 33 + c
    }
    return static_cast<int>(h % static_cast<unsigned long>(capacity));
}

HashMap::HashMap(int initialCapacity)
    : buckets(nullptr), capacity(initialCapacity > 0 ? initialCapacity : 16), size(0) {
    buckets = new Entry[capacity];
}

HashMap::~HashMap() {
    delete[] buckets;
    buckets = nullptr;
}

// Doubles the bucket array and rehashes every live entry.
// Tombstones (occupied && isDeleted) are dropped during rehash, which is
// the natural moment to reclaim them without extra bookkeeping.
void HashMap::resize() {
    int oldCapacity = capacity;
    Entry* oldBuckets = buckets;

    capacity = oldCapacity * 2;
    buckets = new Entry[capacity];
    size = 0; // re-incremented by insert() as we reinsert live entries

    for (int i = 0; i < oldCapacity; ++i) {
        if (oldBuckets[i].occupied && !oldBuckets[i].isDeleted) {
            insert(oldBuckets[i].key, oldBuckets[i].value);
        }
    }

    delete[] oldBuckets;
}

void HashMap::insert(const std::string& key, const CategoryInfo& value) {
    // Load-factor check before insertion. 0.7 is a common upper bound for
    // open addressing with linear probing -- beyond this, probe chains
    // grow long enough to noticeably degrade O(1) lookup behavior.
    if (size >= static_cast<int>(capacity * 0.7)) {
        resize();
    }

    int startIdx = hash(key);
    int firstTombstone = -1;

    for (int i = 0; i < capacity; ++i) {
        int idx = (startIdx + i) % capacity;
        Entry& e = buckets[idx];

        if (!e.occupied) {
            // Truly empty slot: key is definitively not in the table.
            // Prefer to reuse an earlier tombstone if we passed one.
            int target = (firstTombstone != -1) ? firstTombstone : idx;
            buckets[target].key = key;
            buckets[target].value = value;
            buckets[target].occupied = true;
            buckets[target].isDeleted = false;
            ++size;
            return;
        }

        if (e.isDeleted) {
            // Remember the first tombstone but keep probing -- the key may
            // still exist further along the probe chain.
            if (firstTombstone == -1) {
                firstTombstone = idx;
            }
            continue;
        }

        // Slot is live. If keys match, update in place (no size change).
        if (e.key == key) {
            e.value = value;
            return;
        }
    }

    // Table fully traversed without finding key or empty slot.
    // If we recorded a tombstone, we can still place the entry there.
    if (firstTombstone != -1) {
        buckets[firstTombstone].key = key;
        buckets[firstTombstone].value = value;
        buckets[firstTombstone].occupied = true;
        buckets[firstTombstone].isDeleted = false;
        ++size;
        return;
    }

    // Should be unreachable given the load-factor guard above, but resize
    // and retry as a safety net for pathological tombstone-saturation.
    resize();
    insert(key, value);
}

CategoryInfo* HashMap::get(const std::string& key) {
    int startIdx = hash(key);

    for (int i = 0; i < capacity; ++i) {
        int idx = (startIdx + i) % capacity;
        Entry& e = buckets[idx];

        // Truly empty slot terminates the probe -- key cannot be past it.
        if (!e.occupied && !e.isDeleted) {
            return nullptr;
        }

        // Skip tombstones; the key may live further down the chain.
        if (e.isDeleted) {
            continue;
        }

        if (e.key == key) {
            return &e.value;
        }
    }

    return nullptr;
}

bool HashMap::contains(const std::string& key) const {
    int startIdx = hash(key);

    for (int i = 0; i < capacity; ++i) {
        int idx = (startIdx + i) % capacity;
        const Entry& e = buckets[idx];

        if (!e.occupied && !e.isDeleted) {
            return false;
        }

        if (e.isDeleted) {
            continue;
        }

        if (e.key == key) {
            return true;
        }
    }

    return false;
}

// Tombstone deletion: mark the slot deleted but leave occupied=true so
// linear-probe lookups for keys whose probe chain passes through this
// slot do not stop early.
void HashMap::remove(const std::string& key) {
    int startIdx = hash(key);

    for (int i = 0; i < capacity; ++i) {
        int idx = (startIdx + i) % capacity;
        Entry& e = buckets[idx];

        if (!e.occupied && !e.isDeleted) {
            return; // not present
        }

        if (e.isDeleted) {
            continue;
        }

        if (e.key == key) {
            e.isDeleted = true;
            // occupied stays true -- this is the tombstone marker.
            --size;
            return;
        }
    }
}
