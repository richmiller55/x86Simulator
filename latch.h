#ifndef LATCH_H
#define LATCH_H

#include <optional>

template <typename T>
class Latch {
public:
    const std::optional<T>& read() const {
        return current_;
    }

    void write(const T& value) {
        next_ = value;
    }

    void commit() {
        current_ = next_;
        next_.reset();
    }

    void flush() {
        current_.reset();
        next_.reset();
    }

private:
    std::optional<T> current_;
    std::optional<T> next_;
};

#endif // LATCH_H
