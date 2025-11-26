#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>

class GCObject;

// -----------------------------
// GCManager: singleton manager
// -----------------------------
class GCManager {
public:
    GCManager() = default;
    ~GCManager();

    void register_object(GCObject* obj);
    void enqueue_for_deletion(GCObject* obj);

    void collect_cycles();

private:
    void immediate_delete(GCObject* obj);
    void mark(GCObject* obj);
    void sweep();

    std::unordered_set<GCObject*> objects_;
    std::vector<GCObject*> delete_queue_;

    std::unordered_map<GCObject*, bool> markmap_;
};