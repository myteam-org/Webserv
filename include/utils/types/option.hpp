#pragma once
#include <stdexcept>

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
        // もし必要であれば、canUnwrap/unwrap を追加しておくことも可能ですが、
        // 通常はラッパー型で扱うため不要です。
    };

    template<typename T>
    class Option {
        Some<T>* some_;
    public:
        // コピーコンストラクタを public にしておく
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
    Some<T> some(const T& val) { return Some<T>(val); }

	template<typename T>
	Option<T> makeNone() { return Option<T>(None()); }
} // namespace types

