#include "gc_object.hpp"
#include "gc_manager.hpp"

GCObject::GCObject(GCManager& gc) : gc_(gc) {
    gc_.register_object(this);
}

void GCObject::decref() {
    if (--refcount_ == 0) {
        gc_.enqueue_for_deletion(this);
    }
}