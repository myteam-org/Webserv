#pragma once
#include <stdexcept>
#include "http/status.hpp"

namespace types {
    template<typename T>
    struct Some {
    private:
        T val_;
    public:
        explicit Some(T val) : val_(val) {}
        T value() const { return val_; }
    };

    struct None {
    };

    template<typename T>
    class Option {
        Some<T>* some_;
    public:
        Option(const Option& other)
            : some_(other.some_ ? new Some<T>(*other.some_) : 0)
        {}

        Option& operator=(const Option& other) {
            if (this != &other) {
                delete some_;
                some_ = other.some_ ? new Some<T>(*other.some_) : 0;
            }
            return *this;
        }
   
		explicit Option(types::Some<T> some) : some_(new types::Some<T>(some)) {}

		explicit Option(types::None none_val) : some_(0) {
			(void)none_val;
		}

        ~Option() {
            delete some_;
        }

        bool isSome() const {
            return some_ != 0;
        }
        bool isNone() const {
            return some_ == 0;
        }
        T unwrap() const {
            if (isNone()) {
                throw std::runtime_error("Called unwrap on None");
            }
            return some_->value();
        }
        T unwrapOr(const T& default_val) const {
            if (isSome()) {
                return some_->value();
            }
            return default_val;
        }
        // TRYマクロ用のメソッド
        bool canUnwrap() const {
            return isSome();
        }
    };

    template<typename T>
    bool operator==(const Option<T>& lhs, const Option<T>& rhs) {
        if (lhs.isNone() && rhs.isNone()) {
            return true;
        }
        if (lhs.isSome() && rhs.isSome()) {
            return lhs.unwrap() == rhs.unwrap();
        }
        return false;
    }

    template<typename T>
	Option<T> some(const T& val) { return Option<T>(Some<T>(val)); }

	template<typename T>
	Option<T> none() { return Option<T>(None()); }

    types::Option<http::HttpStatusCode> httpStatusCodeFromInt(int code);

    std::string getHttpStatusText(http::HttpStatusCode code);

} // namespace types

// C++ における Rust風の Option<T> 型 を実装したもの
// 値が「ある/Some<T> or ない/None」を安全に扱うための ラッパー構造
// 主な構成
//  Some<T>:値が存在する時のラッパー構造
//  None:値がない時を表す空の構造体
//  Option<T>:Some<T>またはNoneを内部に持ち、値の有無を表すクラス
//  　　　　　　「値があるかないか」を安全に表現するためのテンプレートクラス
//  some(val):Option<T>をSome(val)で作る関数
//  none<T>():Option<T>をNoneで作る関数
// 主なメソッド
//  isSome():値があるか？
//  isNone():値がないか？
//  unwrap():値を取り出す。Noneの時は例外
//  unwrapOr(default):値がなければdefaultを返す
//  canUnwrap():TRYマクロ用:unwrapできるか？
// 使用目的
//  NULLやポインタの存在チェックを安全に行える
//  「値がないことを例外でなく型で表現できる
//  unwrap()で意図的に失敗を設計可能（例外）
