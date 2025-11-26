#include "gc_manager.hpp"
#include "gc_object.hpp"
#include <iostream>

struct MyObj : public GCObject {
    int value;
    MyObj(GCManager& gc, int v) : GCObject(gc), value(v) {}
};

int main() {
    GCManager gc;

    auto* a = new MyObj(gc, 42);
    auto* b = new MyObj(gc, 7);

    a->add_ref(b);
    b->add_ref(a);

    // Drop external references
    a->decref();
    b->decref();

    // Collect cycle
    gc.collect_cycles();

    std::cout << "Done.\n";
}
