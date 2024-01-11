#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }

    size_t DecRef() {
        --count_;
        return count_;
    }

    size_t RefCount() const {
        return count_;
    }

    SimpleCounter& operator=(const SimpleCounter& other) {
        return *this;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : pointer_(nullptr) {
    }

    IntrusivePtr(std::nullptr_t) : pointer_(nullptr) {
    }

    IntrusivePtr(T* ptr) : pointer_(ptr) {
        if (pointer_) {
            pointer_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        pointer_ = other.pointer_;
        if (pointer_) {
            pointer_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        pointer_ = other.pointer_;
        other.pointer_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) {
        pointer_ = other.pointer_;
        if (pointer_) {
            pointer_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) {
        pointer_ = other.pointer_;
        other.pointer_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this == &other) {
            return *this;
        }
        if (pointer_) {
            pointer_->DecRef();
        }
        pointer_ = other.pointer_;
        if (pointer_) {
            pointer_->IncRef();
        }
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this == &other) {
            return *this;
        }
        if (pointer_) {
            pointer_->DecRef();
        }
        pointer_ = other.pointer_;
        other.pointer_ = nullptr;
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (pointer_) {
            pointer_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        if (pointer_) {
            pointer_->DecRef();
        }
        pointer_ = nullptr;
    }

    void Reset(T* ptr) {
        if (pointer_) {
            pointer_->DecRef();
        }
        if (ptr) {
            pointer_ = ptr;
            pointer_->IncRef();
        }
    }

    void Swap(IntrusivePtr& other) {
        std::swap(pointer_, other.pointer_);
    }

    // Observers
    T* Get() const {
        return pointer_;
    }

    T& operator*() const {
        return *pointer_;
    }

    T* operator->() const {
        return pointer_;
    }

    size_t UseCount() const {
        if (!pointer_) {
            return 0;
        }
        return pointer_->RefCount();
    }

    explicit operator bool() const {
        if (pointer_) {
            return true;
        }
        return false;
    }

private:
    T* pointer_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr(new T(std::forward<Args>(args)...));
}
