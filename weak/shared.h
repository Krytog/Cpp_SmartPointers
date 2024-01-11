#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

class BaseBlock {
public:
    virtual size_t GetStrongReferenceCount() = 0;
    virtual void IncreaseStrongReferenceCount() = 0;
    virtual void DecreaseStrongReferenceCount() = 0;

    virtual size_t GetWeakReferenceCount() = 0;
    virtual void IncreaseWeakReferenceCount() = 0;
    virtual void DecreaseWeakReferenceCount() = 0;

    virtual void DestroyObject() = 0;

    void TryDestroy() {
        if (!GetStrongReferenceCount()) {
            DestroyObject();
        }
        if (!GetStrongReferenceCount() && !GetWeakReferenceCount()) {
            delete this;
        }
    }

    virtual ~BaseBlock(){};
};

template <typename T>
class SimpleControlBlock : public BaseBlock {
public:
    SimpleControlBlock() {
        strong_reference_count_ = 0;
        weak_reference_count_ = 0;
        pointer_ = nullptr;
    }

    SimpleControlBlock(T* pointer) {
        strong_reference_count_ = 0;
        weak_reference_count_ = 0;
        pointer_ = pointer;
    }

    size_t GetStrongReferenceCount() override {
        return strong_reference_count_;
    }

    void IncreaseStrongReferenceCount() override {
        ++strong_reference_count_;
    }

    void DecreaseStrongReferenceCount() override {
        --strong_reference_count_;
    }

    size_t GetWeakReferenceCount() override {
        return weak_reference_count_;
    }

    void IncreaseWeakReferenceCount() override {
        ++weak_reference_count_;
    }

    void DecreaseWeakReferenceCount() override {
        --weak_reference_count_;
    }

    void DestroyObject() override {
        if (pointer_) {
            delete pointer_;
        }
        pointer_ = nullptr;
    }

    ~SimpleControlBlock() override {
        if (!strong_reference_count_ && !weak_reference_count_ && pointer_) {
            delete pointer_;
        }
    }

private:
    T* pointer_;
    size_t strong_reference_count_;
    size_t weak_reference_count_;
};

template <typename T>
class ComplexControlBlock : public BaseBlock {
public:
    template <typename... Args>
    ComplexControlBlock(Args&&... args)
        : strong_reference_count_(0),
          weak_reference_count_(0),
          storage_(std::aligned_storage_t<sizeof(T), alignof(T)>()) {
        new (&storage_) T(std::forward<Args>(args)...);
        weak_reference_count_ = 0;
        strong_reference_count_ = 0;
    }

    size_t GetStrongReferenceCount() {
        return strong_reference_count_;
    }

    void IncreaseStrongReferenceCount() {
        ++strong_reference_count_;
    }

    void DecreaseStrongReferenceCount() {
        --strong_reference_count_;
    }

    size_t GetWeakReferenceCount() {
        return weak_reference_count_;
    }

    void IncreaseWeakReferenceCount() {
        ++weak_reference_count_;
    }

    void DecreaseWeakReferenceCount() {
        --weak_reference_count_;
    }

    void DestroyObject() {
        if (!strong_reference_count_ && is_alive_) {
            reinterpret_cast<T*>(&storage_)->~T();
            is_alive_ = false;
        }
    }

    ~ComplexControlBlock() override {
        if (is_alive_) {
            reinterpret_cast<T*>(&storage_)->~T();
        }
    }

    typename std::aligned_storage_t<sizeof(T), alignof(T)>::type* GetStorage() {
        return &storage_;
    }

private:
    size_t strong_reference_count_;
    size_t weak_reference_count_;
    typename std::aligned_storage_t<sizeof(T), alignof(T)>::type storage_;
    bool is_alive_ = true;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : control_block_(nullptr), pointer_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : control_block_(nullptr), pointer_(nullptr) {
    }
    explicit SharedPtr(T* ptr) : control_block_(new SimpleControlBlock<T>(ptr)), pointer_(ptr) {
        Subscribe(control_block_);
    }

    explicit SharedPtr(BaseBlock* control_block) {
        Subscribe(control_block);
    }

    template <typename S>
    explicit SharedPtr(S* ptr) : control_block_(new SimpleControlBlock<S>(ptr)), pointer_(ptr) {
        Subscribe(control_block_);
    }

    template <typename S>
    SharedPtr(const SharedPtr<S> other) {
        Subscribe(other.GetControlBlock());
        pointer_ = other.Get();
    }

    SharedPtr(const SharedPtr& other) {
        Subscribe(other.control_block_);
        pointer_ = other.pointer_;
    }

    SharedPtr(SharedPtr&& other) {
        control_block_ = other.control_block_;
        other.control_block_ = nullptr;
        pointer_ = other.pointer_;
        other.pointer_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        Subscribe(other.GetControlBlock());
        pointer_ = ptr;
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        Subscribe(other.control_block_);
        pointer_ = other.pointer_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (control_block_ == other.control_block_) {
            return *this;
        }
        UnSubscribe();
        Subscribe(other.control_block_);
        pointer_ = other.pointer_;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
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

    ~SharedPtr() {
        UnSubscribe();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        UnSubscribe();
    }

    void Reset(T* ptr) {
        UnSubscribe();
        Subscribe(new SimpleControlBlock<T>(ptr));
        pointer_ = ptr;
    }

    template <typename S>
    void Reset(S* ptr) {
        UnSubscribe();
        Subscribe(new SimpleControlBlock<S>(ptr));
        pointer_ = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(pointer_, other.pointer_);
        std::swap(control_block_, other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
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
        if (!control_block_) {
            return 0;
        }
        return control_block_->GetStrongReferenceCount();
    }

    explicit operator bool() const {
        if (pointer_) {
            return true;
        }
        return false;
    }

    BaseBlock* GetControlBlock() const {
        return control_block_;
    }

    void SetControlBlock(BaseBlock* control_block) {
        control_block_ = control_block;
    }

    void UnSubscribe() {
        pointer_ = nullptr;
        if (control_block_) {
            control_block_->DecreaseStrongReferenceCount();
            control_block_->TryDestroy();
        }
        control_block_ = nullptr;
    }

    void Subscribe(BaseBlock* control_block) {
        control_block_ = control_block;
        if (control_block_) {
            control_block_->IncreaseStrongReferenceCount();
        }
    }

    void SetPointer(T* pointer) {
        pointer_ = pointer;
    }

private:
    BaseBlock* control_block_;
    T* pointer_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ComplexControlBlock<T>* temp = new ComplexControlBlock<T>(std::forward<Args>(args)...);
    auto output = SharedPtr<T>();
    output.Subscribe(temp);
    output.SetPointer(reinterpret_cast<T*>(temp->GetStorage()));
    return output;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
