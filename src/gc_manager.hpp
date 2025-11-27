#pragma once
#include <unordered_set>
#include <vector>
#include <cstddef>

class GCObject;

// -----------------------------
// GCManager: singleton manager
// -----------------------------
class GCManager {
public:
    explicit GCManager(std::size_t generation = 3);
    ~GCManager();

    // registration
    void register_object(GCObject* obj);
    void unregister_object(GCObject* obj);

    // immediate deletion queue (used by decref())
    void enqueue_for_deletion(GCObject* obj);

    // collect generations 0..gen
    void collect(std::size_t gen = 0);

    // tunables
    void set_threshold(std::size_t gen, std::size_t threshold);
    std::size_t get_threshold(std::size_t gen) const;

    // debug / introspection
    std::size_t total_objects() const;
    std::size_t total_in_generation(std::size_t gen) const;

private:
    void immediate_delete(GCObject* obj);
    void collect_trial_deletion(std::size_t gen);
    void promote_survivors(const std::vector<GCObject*>& survivors, std::size_t gen);

    // gen_objects[g] = objects in generation g
    std::vector<std::unordered_set<GCObject*>> gen_objects_;
    std::vector<size_t> thresholds_;
    // count allocations into gen0 to decide collections
    std::vector<size_t> alloc_counts_;

    // immediate delete queue (for non-cyclic immediate destructions)
    std::vector<GCObject*> delete_queue_;
};