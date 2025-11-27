#pragma once
#include <vector>
#include <functional>

class GCManager; // forward declaration

// Base GCObject class
class GCObject {
private:
    GCManager& gc_;
    int refcount_ = 1; // start life with 1 owner by default

public:
    explicit GCObject(GCManager& gc);
    virtual ~GCObject() = default;

    // reference ops
    void incref() { ++refcount_; }
    void decref();

    // traversal for cycle detection: call visitor(child) for each child
    virtual void traverse_references(const std::function<void(GCObject*)>& visitor) {
        (void)visitor;
    }

    // finalizer hook
    virtual bool has_finalizer() const { return false; }
    virtual void finalize() {}

    // metadata exposed for manager use
    int refcount() const { return refcount_; }
    int gc_refs{0};         // used during trial deletion
    bool tracked{false};    // set to true by container types
    int generation{0};      // 0..(max_gen-1)
};