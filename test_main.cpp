#include "gc_object.h"
#include "gc_manager.h"

struct MyObj : public GCObject {
    int value;
    MyObj(int v) : value(v) {}
};

int main() {
    GCManager gc;

    auto* a = new MyObj(42);
    gc.register_object(a);
    auto* b = new MyObj(7);
    gc.register_object(b);
    a->add_ref(b); // create a reference from a to b

    a->decref(); // decrease refcount of a
    gc.collect();

    return 0;
}