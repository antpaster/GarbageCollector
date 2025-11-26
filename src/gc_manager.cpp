#include "gc_manager.hpp"
#include "gc_object.hpp"

GCManager::~GCManager() {
    for (auto* obj : objects_) {
        // Break refcounts
        delete obj;
    }
}

void GCManager::register_object(GCObject* obj) {
    objects_.insert(obj);
}

void GCManager::enqueue_for_deletion(GCObject* obj) {
    delete_queue_.push_back(obj);
}

void GCManager::immediate_delete(GCObject* obj) {
    for (auto* child : obj->children()) {
        child->decref();
    }
    objects_.erase(obj);
    delete obj;
}

void GCManager::mark(GCObject* obj) {
    if (markmap_[obj])
        return;
    markmap_[obj] = true;
    for (auto* c : obj->children()) {
        mark(c);
    }
}

void GCManager::collect_cycles() {
    // 1. Immediate refcount-based deletions
    for (auto* obj : delete_queue_) {
        immediate_delete(obj);
    }
    delete_queue_.clear();

    // 2. Mark phase
    markmap_.clear();
    for (auto* obj : objects_) {
        if (obj->refcount() > 0)
            mark(obj);
    }

    // 3. Sweep unreachable
    sweep();
}

void GCManager::sweep() {
    std::vector<GCObject*> to_delete;
    for (auto* obj : objects_) {
        if (!markmap_[obj])
            to_delete.push_back(obj);
    }
    for (auto* obj : to_delete) {
        immediate_delete(obj);
    }
}
