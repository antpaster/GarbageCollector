#pragma once
#include <functional>

// Base GCObject class
class GCObject {
public:
    GCObject() = default;
    virtual ~GCObject() = default;

    // Intrusive refcount: starts at 0; owners (roots or other objects) call incref.
    int refcount{0};

    // For cycle detection bookeeeping
    int gc_refs{0};

    // Whether the object can hold references to other GC objects and should be tracked
    bool tracked{false};

    // By default no finalizer
    virtual bool has_finalizer() const { return false; }
    virtual void finalize() {}

    // Subclasses that contain pointers must implement traversal:
    // call visitor(child) for each child pointer (may be null).
    virtual void traverse_references(const std::function<void(GCObject*)>& visitor) {
        (void)visitor;
    }

    // Helpers
    void incref() {
        ++refcount;
    }

    void decref() {
        --refcount;
        if (refcount == 0) {
            // immediate destruction (non-cyclic case)
            // Ensure unregister before delete to keep manager consistent.
            GCManager::instance().unregister_object(this);
            delete this;
        }
    }
};