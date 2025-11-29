#include <gtest/gtest.h>
#include "gc_manager.hpp"
#include "gc_object.hpp"

// A simple tracked object type for tests
struct TObj : public GCObject {
    const char* label;
    std::vector<GCObject*> children;

    explicit TObj(GCManager& gc, const char* label = nullptr) : GCObject(gc), label(label) {
        tracked = true;
    }

    void traverse_references(const std::function<void(GCObject*)>& visitor) override {
        for (auto* c : children) {
            visitor(c);
        }
    }
};

// 1. Immediate deletion test
TEST(GC_ImmediateDeletion, NonCyclicRefcountZero) {
    GCManager gc(3);

    auto* a = new TObj(gc, "a"); // starts with refcount = 1
    a->decref(); // refcount becomes 0 -> immediate deletion

    // collection shouldn't crash and a should be gone
    gc.collect(0);

    EXPECT_EQ(gc.total_objects(), 0u);
}

// 2. Cycle cleanup test
TEST(GC_CycleCollection, SimpleCycle) {
    GCManager gc(3);

    auto* a = new TObj(gc, "a");
    auto* b = new TObj(gc, "b");

    a->children.push_back(b);
    b->children.push_back(a);

    // remove external ownership
    a->decref();
    b->decref();

    // should be collected when collecting gen0
    gc.collect(0);

    EXPECT_EQ(gc.total_objects(), 0u);
}

// TEST(GCCycles, DetectCycle) {
//     GCManager gc;

//     auto* a = new TestObj(gc);
//     auto* b = new TestObj(gc);

//     a->add_ref(b);
//     b->add_ref(a);

//     a->decref();
//     b->decref();

//     gc.collect_cycles();

//     SUCCEED();
// }
