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
        Ok<T>* ok_;
        Err<E>* err_;

    public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(types::Ok<T> ok) : ok_(new types::Ok<T>(ok)), err_(NULL) {}
        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(types::Err<E> err) : ok_(NULL), err_(new types::Err<E>(err)) {}

        ~Result() {
            delete ok_;
            delete err_;
        }

        bool isOk() const { return ok_ != NULL; }
        bool isErr() const { return err_ != NULL; }

        T unwrap() const {
            if (!isOk()) {
                throw std::runtime_error("Called unwrap on Err");
            }
            return ok_->value();
        }

        E unwrapErr() const {
            if (!isErr()) {
                 throw std::runtime_error("Called unwrap_err on Ok");
            }
            return err_->error();
        }

	    bool canUnwrap() const {
			return isOk();
		}
    };

    template<typename T>
    Ok<T> ok(const T& val) { return Ok<T>(val); }

    template<typename E>
    Err<E> err(const E& error) { return Err<E>(error); }
} // namespace types
