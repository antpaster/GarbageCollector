#pragma once
#include <vector>
#include <iostream>

class GCObject {
public:
    int refcount = 0;
    std::vector<GCObject*> refs;

    virtual ~GCObject() = default;

    void add_ref(GCObject* obj) {
        if (obj) {
            refs.push_back(obj);
            obj->incref();
        }
    }

    void incref() {
        refcount++;
    }
    void decref() {
        if (--refcount == 0) {
            destroy();
        }
    }

protected:
    virtual void destroy() {
        for (auto ref : refs) {
            ref->decref();
        }
        refs.clear();
        delete this;
    }
};