#include <iostream>
#include <queue>
#include <unordered_set>
#include <algorithm>

#include "gc_manager.hpp"
#include "gc_object.hpp"

GCManager::GCManager(std::size_t generations) :
    gen_objects_(generations),
    thresholds_(generations, 50),
    alloc_counts_(generations, 0)
{}  

GCManager::~GCManager() {
    // delete everything
    for (auto& genset : gen_objects_) {
        for (auto* obj : genset) {
            delete obj;
        }
        genset.clear();
    }
}

void GCManager::register_object(GCObject* obj) {
    obj->generation = 0;
    gen_objects_[0].insert(obj);
    ++alloc_counts_[0];

    // if gen0 exceeds threashold, collect gen0
    if (alloc_counts_[0] >= thresholds_[0]) {
        collect(0);
        alloc_counts_[0] = 0;
    }
}

void GCManager::unregister_object(GCObject* obj) {
    // remove from whichever generation set holds it (if present)
    auto g = static_cast<std::size_t>(std::max(0, obj->generation));
    if (g >= gen_objects_.size()) {
        g = gen_objects_.size() - 1;
    }
    gen_objects_[g].erase(obj);
}

void GCManager::enqueue_for_deletion(GCObject* obj) {
    delete_queue_.push_back(obj);
}

void GCManager::immediate_delete(GCObject* obj) {
    // decrement children refs then delete
    obj->traverse_references([&](GCObject* c) {
        if (c) {
            c->decref();
        }
    });
    // remove from its generation set to keep manager consistent
    auto g = static_cast<std::size_t>(std::max(0, obj->generation));
    if (g >= gen_objects_.size()) {
        g = gen_objects_.size() - 1;
    }
    gen_objects_[g].erase(obj);
    delete obj;
}

void GCManager::collect(std::size_t gen) {
    if (gen >= gen_objects_.size()) {
        gen = gen_objects_.size() - 1;
    }

    // 1. handle immediate deletions first (non-cyclic)
    for (auto* o : delete_queue_) {
        immediate_delete(o);
    }
    delete_queue_.clear();

    // 2. perform trail-deletion on tracked objects in generations 0..gen
    collect_trial_deletion(gen);
}

void GCManager::collect_trial_deletion(std::size_t gen) {
    // Build L: all tracked objects in gens 0..gen
    std::vector<GCObject*> L;
    L.reserve(256);
    std::unordered_set<GCObject*> Lset;
    for (std::size_t g = 0; g <= gen; ++g) {
        for (auto* o : gen_objects_[g]) {
            if (o->tracked) {
                L.push_back(o);
                Lset.insert(o);
            }
        }
    }

    if (L.empty()) {
        // nothing to do
        return;
    }

    // init gc_refs := refcount
    for (auto* o : L) o->gc_refs = o->refcount();

    // subtract internal references
    for (auto* o : L) {
        o->traverse_references([&](GCObject* c) {
            if (c && Lset.count(c)) {
                --(c->gc_refs);
            }
        });
    }

    // queue candidates with gc_refs == 0
    std::queue<GCObject*> Q;
    for (auto* o : L) if (o->gc_refs == 0) Q.push(o);

    // propagate deletions
    while (!Q.empty()) {
        auto* o = Q.front(); Q.pop();
        o->traverse_references([&](GCObject* c) {
            if (c && Lset.count(c)) {
                --(c->gc_refs);
                if (c->gc_refs == 0) Q.push(c);
            }
        });
    }

    // collect unreachable (gc_refs == 0) and survivors (gc_refs > 0)
    std::vector<GCObject*> unreachable;
    std::vector<GCObject*> survivors;
    unreachable.reserve(L.size()/4);
    survivors.reserve(L.size());

    for (auto* o : L) {
        if (o->gc_refs == 0) unreachable.push_back(o);
        else survivors.push_back(o);
    }

    // finalize unreachable (they may resurrect)
    for (auto* o : unreachable) {
        if (o->has_finalizer()) o->finalize();
    }

    // filter out resurrected objects (refcount > 0) — move them to survivors
    std::vector<GCObject*> to_destroy;
    to_destroy.reserve(unreachable.size());
    for (auto* o : unreachable) {
        if (o->refcount() > 0) survivors.push_back(o);
        else to_destroy.push_back(o);
    }

    // destroy remaining unreachable objects
    for (auto* o : to_destroy) {
        // remove from generation container and delete (immediate_delete will decref children)
        std::size_t g = static_cast<std::size_t>(std::max(0, o->generation));
        if (g >= gen_objects_.size()) g = gen_objects_.size()-1;
        gen_objects_[g].erase(o);

        // call deletion logic directly (children decref -> may enqueue further immediate deletions)
        o->traverse_references([&](GCObject* c) {
            if (c) c->decref();
        });
        delete o;
    }

    // promote survivors that were in gens <= gen and haven't reached max generation
    promote_survivors(survivors, gen);
}

void GCManager::promote_survivors(const std::vector<GCObject*>& survivors, std::size_t gen) {
    std::size_t max_gen = gen_objects_.size() - 1;
    for (auto* o : survivors) {
        std::size_t g = static_cast<std::size_t>(std::max(0, o->generation));
        if (g < max_gen && g <= gen) {
            // move object into next generation
            gen_objects_[g].erase(o);
            ++(o->generation);
            gen_objects_[o->generation].insert(o);
        }
    }
}

void GCManager::set_threshold(std::size_t gen, std::size_t threshold) {
    if (gen < thresholds_.size()) thresholds_[gen] = threshold;
}

std::size_t GCManager::get_threshold(std::size_t gen) const {
    if (gen < thresholds_.size()) return thresholds_[gen];
    return 0;
}

std::size_t GCManager::total_objects() const {
    std::size_t s = 0;
    for (const auto& g : gen_objects_) s += g.size();
    return s;
}

std::size_t GCManager::total_in_generation(std::size_t gen) const {
    if (gen < gen_objects_.size()) return gen_objects_[gen].size();
    return 0;
}