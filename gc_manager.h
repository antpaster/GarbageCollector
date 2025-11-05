#pragma once
#include <unordered_set>

class GCManager {
    std::unordered_set<GCObject*> all_objects;

public:
    void register_object(GCObject* obj) {
        all_objects.insert(obj);
    }

    void collect() {
        std::cout << "[GC] Objects in heap: " << all_objects.size() << std::endl;
    }
};