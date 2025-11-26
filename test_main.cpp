#include "gc_object.h"
#include "gc_manager.h"
#include "gc_root.h"

int main() {
    auto& gc = GCManager::instance();

    std::cout << "=== Test 1: acyclic release frees immediately ===\n";
    {
        // create objects
        Obj* a = gc.make<Obj>("A");
        Obj* b = gc.make<Obj>("B");

        // root holds A
        GCRoot<Obj> rA(a); // rA owns A (incref)
        // A -> B
        a->add_child(b);

        std::cout << "[State] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";

        // release root; this should delete A and (during A's destroy) B (because only A owned B)
        rA.reset();

        std::cout << "[After reset] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";
    }

    std::cout << "\n=== Test 2: simple cycle collected by GC ===\n";
    {
        Obj* a = gc.make<Obj>("C1");
        Obj* b = gc.make<Obj>("C2");

        // Use roots to own them temporarily
        GCRoot<Obj> r1(a), r2(b);

        // create cycle C1 -> C2 -> C1
        a->add_child(b);
        b->add_child(a);

        // release external roots -> only cycle references remain
        r1.reset();
        r2.reset();

        std::cout << "[Before collect] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";

        // Now run collector; it should detect the cycle and delete both objects
        gc.collect();

        std::cout << "[After collect] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";
    }

    std::cout << "\n=== Test 3: finalizer resurrection ===\n";
    {
        Obj* a = gc.make<Obj>("R1");
        Obj* b = gc.make<Obj>("R2");

        GCRoot<Obj> r1(a), r2(b);

        // create cycle R1 <-> R2
        a->add_child(b);
        b->add_child(a);

        // Mark R1's finalizer to resurrect itself when finalized
        a->run_finalizer_on_collect = true;
        a->resurrect_in_finalizer = true;

        // drop external roots
        r1.reset();
        r2.reset();

        std::cout << "[Before collect] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";

        // run collector: finalizer of R1 will resurrect R1 (incref), so it should not be destroyed
        gc.collect();

        std::cout << "[After collect] total objects: " << gc.total_objects() << ", tracked: " << gc.total_tracked() << "\n";

        // Clean up: if resurrected, we should hold a root to release it
        // Find remaining objects and release by hand (for demo only)
        // Note: in a real program you'd keep better handles; here we brute-force decref remaining objects.
        // Gather all remaining objects and decref them to clean up.
        // (We cannot access GCManager's internals here — so for demo we stop.)
        std::cout << "[Demo] End of tests. (Objects that were resurrected remain alive until process exit or cleaned by further code.)\n";
    }

    return 0;
}