#include <iostream>
#include <string>

#include "gc_manager.hpp"
#include "gc_object.hpp"

// Concrete object type that holds children (tracked)
struct Obj : public GCObject {
    Obj(GCManager& gc, const std::string& name) : GCObject(gc), name(name) {
        tracked = true;
    }
    ~Obj() override {
        std::cout << "[Dtor] " << name << " (gen " << generation << ")\n";
    }
    void traverse_references(const std::function<void(GCObject*)>& visitor) override {
        for (auto* c : children) visitor(c);
    }
    std::string name;
    std::vector<GCObject*> children;
};

int main() {
    GCManager gc(3);
    gc.set_threshold(0, 3); // small threshold to trigger collections sooner for demo

    // Create many short-lived objects to show promotions
    for (int i = 0; i < 6; ++i) {
        Obj* a = new Obj(gc, "o" + std::to_string(i));
        // Simulate short-lived external owner releasing quickly:
        a->decref(); // if no other owners, immediate deletion happens
    }

    // Create survivor object that will be promoted
    Obj* p = new Obj(gc, "persistent");
    p->incref(); // hold onto it externally so it doesn't get deleted immediately

    // Add a child to keep it alive across collections
    for (int i = 0; i < 4; ++i) {
        Obj* child = new Obj(gc, "child" + std::to_string(i));
        p->children.push_back(child);
        child->incref();
        child->decref(); // release external owner; child still owned by p
    }

    // Force multiple collections to promote 'p' upward
    gc.collect(0); // collect gen0
    gc.collect(0);
    gc.collect(1); // collect gen1 (which includes objects promoted from gen0)
    gc.collect(2); // perhaps collect old generation

    // Release persistent root
    p->decref();

    // Final collection to clean remaining cycles if any
    gc.collect(2);

    std::cout << "Final total objects: " << gc.total_objects() << "\n";
    return 0;
}
