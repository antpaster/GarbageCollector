#pragma once
#include <vector>

class GCManager; // forward declaration

// Base GCObject class
class GCObject {
private:
    friend class GCManager;

    GCManager& gc_;
    int refcount_ = 1;
    std::vector<GCObject*> children_;

protected:
    explicit GCObject(GCManager& gc);

public:
    virtual ~GCObject() = default;

    void add_ref(GCObject* obj);
    void incref() { ++refcount_; }
    void decref();

    const std::vector<GCObject*>& children() const { return children_; }
    int refcount() const { return refcount_; }
};