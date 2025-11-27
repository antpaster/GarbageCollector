#include <gtest/gtest.h>
#include "gc_manager.hpp"
#include "gc_object.hpp"

// struct TestObj : public GCObject {
//     TestObj(GCManager& gc) : GCObject(gc) {}
// };

// TEST(GCSimple, SingleDeletion) {
//     GCManager gc;

//     auto* a = new TestObj(gc);
//     a->decref(); // should be deleted

//     gc.collect_cycles();

//     SUCCEED();
// }

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
