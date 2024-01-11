#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <typename T>
struct Slug {
    Slug() = default;

    template <typename S>
    Slug(const S& dummy) {
    }

    void operator()(T* pointer) {
        delete pointer;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    template <typename S>
    Slug(const S& dummy) {
    }

    void operator()(T* pointer) {
        delete[] pointer;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    template <typename S, typename S_Deleter>
    friend class UniquePtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : self_(ptr, Deleter{}) {
    }
    UniquePtr(T* ptr, Deleter deleter) : self_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        self_ = CompressedPair<T*, Deleter>(other.self_.GetFirst(),
                                            std::forward<Deleter>(other.self_.GetSecond()));
        other.self_.GetFirst() = nullptr;
    }

    template <typename S, typename S_Deleter = Deleter>
    UniquePtr(UniquePtr<S, S_Deleter>&& other) {
        self_ = CompressedPair<T*, Deleter>(other.self_.GetFirst(),
                                            std::forward<S_Deleter>(other.self_.GetSecond()));
        self_.GetFirst() = other.self_.GetFirst();
        other.self_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        self_.GetSecond()(self_.GetFirst());
        self_.GetFirst() = other.self_.GetFirst();
        self_.GetSecond() = std::forward<Deleter>(other.self_.GetSecond());
        other.self_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        self_.GetSecond()(self_.GetFirst());
        self_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        self_.GetSecond()(self_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto temp = self_.GetFirst();
        self_.GetFirst() = nullptr;
        return temp;
    }

    void Reset(T* ptr = nullptr) {
        auto temp = self_.GetFirst();
        self_.GetFirst() = ptr;
        if (temp) {
            self_.GetSecond()(temp);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(self_.GetFirst(), other.self_.GetFirst());
        std::swap(self_.GetSecond(), other.self_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return self_.GetFirst();
    }

    Deleter& GetDeleter() {
        return self_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return self_.GetSecond();
    }

    explicit operator bool() const {
        if (self_.GetFirst()) {
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *self_.GetFirst();
    }

    T* operator->() const {
        return self_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> self_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : self_(ptr, Deleter{}) {
    }
    UniquePtr(T* ptr, Deleter deleter) : self_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        self_ = CompressedPair<T*, Deleter>(other.self_.GetFirst(),
                                            std::forward<Deleter>(other.self_.GetSecond()));
        other.self_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        self_.GetSecond()(self_.GetFirst());
        self_.GetFirst() = other.self_.GetFirst();
        self_.GetSecond() = std::forward<Deleter>(other.self_.GetSecond());
        other.self_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        self_.GetSecond()(self_.GetFirst());
        self_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        self_.GetSecond()(self_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto temp = self_.GetFirst();
        self_.GetFirst() = nullptr;
        return temp;
    }

    void Reset(T* ptr = nullptr) {
        auto temp = self_.GetFirst();
        self_.GetFirst() = ptr;
        if (temp) {
            self_.GetSecond()(temp);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(self_.GetFirst(), other.self_.GetFirst());
        std::swap(self_.GetSecond(), other.self_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return self_.GetFirst();
    }

    Deleter& GetDeleter() {
        return self_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return self_.GetSecond();
    }

    explicit operator bool() const {
        if (self_.GetFirst()) {
            return true;
        }
        return false;
    }

    std::add_lvalue_reference_t<T> operator[](size_t pos) {
        return *(self_.GetFirst() + pos);
    }

private:
    CompressedPair<T*, Deleter> self_;
};
