// RAII owning handle for roots
template <typename T>
class GCRoot {
private:
    T* ptr;

public:
    GCRoot() : ptr(nullptr) {}
    explicit GCRoot(T* p) : ptr(p) {
        if (ptr) {
            ptr->incref();
        }
    }
    GCRoot(const GCRoot& other) : ptr(other.ptr) {
        if (ptr) {
            ptr->incref();
        }
    }
    GCRoot& operator=(const GCRoot& other) {
        if (this == &other) {
            return *this;
        }
        reset();
        ptr = other.ptr;
        if (ptr) {
            ptr->incref();
        }
        return *this;
    }
    GCRoot(GCRoot&& o) noexcept : ptr(o.ptr) {
        o.ptr = nullptr;
    }
    GCRoot& operator=(GCRoot&& o) noexcept {
        if (this == &o) {
            return *this;
        }
        reset();
        ptr = o.ptr;
        o.ptr = nullptr;
        return *this;
    }
    ~GCRoot() {
        reset();
    }

    void reset() {
        if (ptr) {
            ptr->decref(); // may delete of reaches 0
            ptr = nullptr;
        }
    }

    T* get() const { return ptr; }
    T* operator->() const { return ptr; }
    explicit operator bool() const { return ptr != nullptr; }
};