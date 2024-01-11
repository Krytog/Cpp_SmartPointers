#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename S>
    friend class SharedPtr;

    template <typename S>
    friend class EnableSharedFromThis;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : control_block_(nullptr), pointer_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) {
        Subscribe(other.control_block_);
        pointer_ = other.pointer_;
    }

    WeakPtr(WeakPtr&& other) {
        control_block_ = other.control_block_;
        pointer_ = other.pointer_;
        other.control_block_ = nullptr;
        other.pointer_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        Subscribe(other.GetControlBlock());
        pointer_ = other.Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (control_block_ == other.control_block_) {
            return *this;
        }
        UnSubscribe();
        Subscribe(other.control_block_);
        pointer_ = other.pointer_;
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (control_block_ == other.control_block_) {
            return *this;
        }
        UnSubscribe();
        std::swap(control_block_, other.control_block_);
        std::swap(pointer_, other.pointer_);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        UnSubscribe();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        UnSubscribe();
    }

    void Swap(WeakPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(pointer_, other.pointer_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!control_block_) {
            return 0;
        }
        return control_block_->GetStrongReferenceCount();
    }

    bool Expired() const {
        if (!control_block_) {
            return true;
        }
        if (control_block_->GetStrongReferenceCount()) {
            return false;
        }
        return true;
    }

    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        SharedPtr<T> output;
        output.Subscribe(control_block_);
        output.SetPointer(pointer_);
        return output;
    }

    BaseBlock* GetControlBlock() {
        return control_block_;
    }

private:
    void UnSubscribe() {
        pointer_ = nullptr;
        if (control_block_) {
            control_block_->DecreaseWeakReferenceCount();
            control_block_->TryDestroy();
        }
        control_block_ = nullptr;
    }

    void Subscribe(BaseBlock* control_block) {
        control_block_ = control_block;
        if (control_block_) {
            control_block_->IncreaseWeakReferenceCount();
        }
    }

    BaseBlock* control_block_;
    T* pointer_;
};
