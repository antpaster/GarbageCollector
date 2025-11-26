#pragma once
#include <functional>
#include <string>
#include <vector>
#include <iostream>

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

class Obj : public GCObject {
public:
    std::string name;
    std::vector<GCObject*> children;
    bool run_finalizer_on_collect = false;
    bool resurrect_in_finalizer = false;

    Obj(const std::string& n) : name(n) {
        tracked = true; // this class holds references, track it
    }

    ~Obj() override {
        std::cout << "[Dtor] " << name << std::endl;
    }

    void add_child(GCObject* c) {
        if (!c) {
            return;
        }
        children.push_back(c);
        c->incref();
    }

    void traverse_references(const std::function<void(GCObject*)>& visitor) override {
        for (auto* c : children) {
            visitor(c);
        }
    }

    bool has_finalizer() const override {
        return run_finalizer_on_collect;
    }

    void finalize() override {
        std::cout << "[Finalize] " << name << std::endl;
        if (resurrect_in_finalizer) {
            std::cout << "[Finalize] resurrecting " << name << "by incref\n";
            this->incref(); // resurrect: increase refcount so GC will treat as alive
        }
    }
};