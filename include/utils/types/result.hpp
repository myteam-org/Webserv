#pragma once

#include <stdexcept> // std::runtime_error 用

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

namespace types {

    struct Unit {}; // Rustに相当

    template<typename T, typename E>
    class Result {
        Ok<T>* ok_;
        Err<E>* err_;

    public:
        // コピーコンストラクタ
        Result(const Result& other)
            : ok_(other.ok_ ? new Ok<T>(*other.ok_) : NULL),
              err_(other.err_ ? new Err<E>(*other.err_) : NULL) {}

        // 代入演算子
        Result& operator=(const Result& other) {
            if (this != &other) {
                delete ok_;
                delete err_;
                ok_ = other.ok_ ? new Ok<T>(*other.ok_) : NULL;
                err_ = other.err_ ? new Err<E>(*other.err_) : NULL;
            }
            return *this;
        }

        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(Ok<T> ok) : ok_(new Ok<T>(ok)), err_(NULL) {}
        // NOLINTNEXTLINE(google-explicit-constructor)
        Result(Err<E> err) : ok_(NULL), err_(new Err<E>(err)) {}

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
// #define OK(val) ::types::ok(val)
// #define ERR(e) ::types::err(e)
#define OK(val) (::types::ok(val))
#define ERR(e)  (::types::err(e))
#define RETURN_IF_ERR(expr) \
    do { \
        const types::Result<types::Unit, error::AppError> _res = (expr); \
        if (_res.isErr()) \
            return ERR(_res.unwrapErr()); \
    } while (0)

// 「成功」「失敗」を型で安全に扱うための仕組み
// RustのResult<T, E>型をC++で再現した実装
// ＜クラス＞
//  Ok<T>:成功時の値を持つ（成功のラッパー）
//  Err<E>:失敗（エラー）を持つ（失敗のラッパー）
//  Result<T, E>:成功か失敗のどちらかを保持する型
//               Ok<T>やErr<E>で包みunwrap()などで安全に扱うことができる
//               Ok<T>*,Err<E>*のいずれかを持つ
// ＜メソッド＞
//  ok(val):Ok<T>を作る関数（マクロでも使用）
//  err(e):Err<E>を作る関数（マクロでも使用）
//  OK(val):ok(val)マクロ
//  ERR(e):err(e)のマクロ
//  isOk():成功か？
//  isErr():失敗か？
//  unwrap():成功時の値を返す。失敗なら例外。
//  unwrapErr():失敗時の値を返す。成功なら例外。
//  canUnwrap():TRYマクロ用のisOk()エイリアス
// 関数の返り値で「成功/失敗」を型で表現できる
// unwrapで失敗時に強制的に例外を出すことができる
