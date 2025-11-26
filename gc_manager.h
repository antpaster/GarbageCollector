#pragma once
#include <unordered_set>
#include <queue>

#include "gc_object.h"

// -----------------------------
// GCManager: singleton manager
// -----------------------------
class GCManager {
private:
    GCManager() = default;
    std::unordered_set<GCObject*> all_objects;
    std::unordered_set<GCObject*> tracked_objects;

public:
    static GCManager& instance() {
        static GCManager instance;
        return instance;
    }

    // Factory: constructs object, registers it, returns raw pointer
    template<typename T, typename... Args>
    T* make(Args&&... args) {
        T* obj = new T(std::forward<Args>(args)...);
        register_object(obj);
        return obj;
    }

    void register_object(GCObject* obj) {
        all_objects.insert(obj);
        if (obj->tracked) {
            tracked_objects.insert(obj);
        }
    }

    void unregister_object(GCObject* obj) {
        all_objects.erase(obj);
        tracked_objects.erase(obj);
    }

    size_t total_objects() const {
        return all_objects.size();
    }
    size_t total_tracked() const {
        return tracked_objects.size();
    }

    // The CPython trial-deletion collector for generation 0 (single-gen here)
    void collect() {
        std::vector<GCObject*> L;
        L.reserve(tracked_objects.size());

        // 1) gather tracked objects
        for (auto* o : tracked_objects) {
            L.push_back(o);
        }

        if (L.empty()) {
            std::cout << "[GC] No tracked objects to collect.\n";
            return;
        }

        // Build quick lookup set
        std::unordered_set<GCObject*> Lset(L.begin(), L.end());

        // 2) initialize gc_refs = refcount
        for (auto* o : L) {
            o->gc_refs = o->refcount;
        }

        // 3) subtract internal references
        for (auto* o : L) {
            o->traverse_references([&](GCObject* c) {
                if (c && Lset.count(c)) {
                    --(c->gc_refs);
                }
            });
        }

        // 4) queue objects with gc_refs == 0
        std::queue<GCObject*> Q;
        for (auto* o : L) {
            if (o->gc_refs == 0) {
                Q.push(o);
            }
        }

        // 5) propagate deletable
        while (!Q.empty()) {
            GCObject* o = Q.front();
            Q.pop();
            o->traverse_references([&](GCObject* c) {
                if (c && Lset.count(c)) {
                    --(c->gc_refs);
                    if (c->gc_refs == 0) {
                        Q.push(c);
                    }
                }
            });
        }

         // 6) collect unreachable (gc_refs == 0) candidates
         std::vector<GCObject*> unreachable;
         std::vector<GCObject*> reachable;
        for (auto* o : L) {
            if (o->gc_refs == 0) {
                unreachable.push_back(o);
            } else {
                reachable.push_back(o);
            }
        }

        std::cout << "[GC] Tracked: " << L.size() << ", reachable" << reachable.size()
                  << ", unreachable (candidates): " << unreachable.size() << "\n";

        if (unreachable.empty()) {
            return;
        }

        // 7) run finalizer for unreachable (they may resurrect objects by incref)
        for (auto* o : unreachable) {
            if (o->has_finalizer()) {
                o->finalize();
            }
        }

        // 8) detect resurrection: objects in unreachable that gained refcount > 0
        std::vector<GCObject*> to_destroy;
        std::vector<GCObject*> resurrected;
        for (auto* o : unreachable) {
            if (o->refcount > 0) {
                resurrected.push_back(o);
            } else {
                to_destroy.push_back(o);
            }
        }

        if (!resurrected.empty()) {
            std::cout << "[GC] Resurrected objects during finalization: " << resurrected.size() << "\n";
        }

        // 9) destroy (unregister + delete) the remaining unreachable objects
        for (auto* o : to_destroy) {
            // careful: destroy will also decrement child refs (and may recursively destroy)
            unregister_object(o); // remove the manager before deletion
            delete o;
        }
    }
};