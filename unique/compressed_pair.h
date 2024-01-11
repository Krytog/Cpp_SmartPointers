#pragma once

#pragma once

#include <type_traits>
#include <memory>

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S>
inline constexpr int8_t EboCase() {
    if ((!std::is_empty_v<F> && !std::is_empty_v<S>)) {
        return 0;
    }
    if (std::is_empty_v<F> && !std::is_empty_v<S>) {
        if (!std::is_final_v<F>) {
            return 1;
        } else {
            return 0;
        }
    }
    if (!std::is_empty_v<F> && std::is_empty_v<S>) {
        if (!std::is_final_v<S>) {
            return 2;
        } else {
            return 0;
        }
    }
    if (std::is_empty_v<F> && std::is_empty_v<S>) {
        if (std::is_base_of_v<F, S>) {
            return 4;
        }
        if (std::is_base_of_v<S, F>) {
            return 5;
        }
        if (!std::is_final_v<F> && !std::is_final_v<S>) {
            return 3;
        } else if (!std::is_final_v<F> && std::is_final_v<S>) {
            return 1;
        } else if (std::is_final_v<F> && !std::is_final_v<S>) {
            return 2;
        } else {
            return 0;
        }
    }
}

template <typename F, typename S, int8_t ebo_case = EboCase<F, S>()>
class CompressedPair {};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(0)> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(1)> : public F {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : F(first), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : F(first), second_(std::move(second)) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(2)> : public S {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), S(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(second) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(3)> : public F, public S {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second) {
    }
    CompressedPair(const F& first, S&& second) {
    }
    CompressedPair(F&& first, const S& second) {
    }
    CompressedPair(F&& first, S&& second) {
    }

    F& GetFirst() {
        return static_cast<F>(*this);
    }

    const F& GetFirst() const {
        return static_cast<F>(*this);
    }

    S& GetSecond() {
        return static_cast<S>(*this);
    };

    const S& GetSecond() const {
        return static_cast<S>(*this);
    };
};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(4)> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, static_cast<int8_t>(5)> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) : first_(first), second_(second) {
    }
    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};
