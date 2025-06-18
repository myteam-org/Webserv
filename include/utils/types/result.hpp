#pragma once

#include <stdexcept> // std::runtime_error 用

namespace types {
    template<typename T>
    struct Ok {
    private:
        T val_;

    public:
        explicit Ok(T val) : val_(val) {}
        T value() const { return val_; } // getter
    };

    template<typename E>
    struct Err {
    private:
        E err_;

    public:
        explicit Err(E err) : err_(err) {}
        E error() const { return err_; } // getter
    };

    template<typename T, typename E>
    class Result {
        Ok<T>* ok_;   // Ok 状態の場合はここに値が入る
        Err<E>* err_; // Err 状態の場合はここに値が入る

    public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(types::Ok<T> ok) : ok_(new types::Ok<T>(ok)), err_(NULL) {}
        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(types::Err<E> err) : ok_(NULL), err_(new types::Err<E>(err)) {}

        ~Result() {
            delete ok_;
            delete err_;
        }

        bool is_ok() const { return ok_ != NULL; }
        bool is_err() const { return err_ != NULL; }

        T unwrap() const {
            if (!is_ok()) {
                throw std::runtime_error("Called unwrap on Err");
            }
            return ok_->value(); // getter を使用
        }

        E unwrap_err() const {
            if (!is_err()) {
                 throw std::runtime_error("Called unwrap_err on Ok");
            }
            return err_->error(); // getter を使用
        }
    };

    template<typename T>
    Ok<T> ok(const T& val) { return Ok<T>(val); }

    template<typename E>
    Err<E> err(const E& error) { return Err<E>(error); }
} // namespace types
