#pragma once

#include <stdexcept>

namespace types {
    template <class T>
    class Left {
    public:
        explicit Left(const T& val) : val_(val) {}
        Left(const Left& other) : val_(other.val_) {}
        
        T val() const { return val_; }

    private:
        T val_;
    };

    template <class T>
    class Right {
    public:
        explicit Right(const T& val) : val_(val) {}
        Right(const Right& other) : val_(other.val_) {}
        
        T val() const { return val_; }

    private:
        T val_;
    };
} //namespace types

template <class T>
types::Left<T> Left(const T& val) {
    return types::Left<T>(val);
}

template <class T>
types::Right<T> Right(const T& val) {
    return types::Right<T>(val);
}

template <class L, class R>
class Either {
public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(const types::Left<L>& left) : left_(new types::Left<L>(left)), right_(NULL) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(const types::Right<R>& right) : left_(NULL), right_(new types::Right<R>(right)) {}
    
    Either(const Either& other) {
        if (other.left_) {
            left_ = new types::Left<L>(*other.left_);
            right_ = NULL;
        } else {
            left_ = NULL;
            right_ = new types::Right<R>(*other.right_);
        }
    }
    
    ~Either() {
        delete left_;
        delete right_;
    }
    
    Either& operator=(const Either& other) {
        if (this != &other) {
            delete left_;
            delete right_;
            
            if (other.left_) {
                left_ = new types::Left<L>(*other.left_);
                right_ = NULL;
            } else {
                left_ = NULL;
                right_ = new types::Right<R>(*other.right_);
            }
        }
        return *this;
    }
    
    bool isLeft() const { return left_ != NULL; }
    bool isRight() const { return right_ != NULL; }
    
    L unwrapLeft() const {
        if (!left_) {
            throw std::runtime_error("called unwrapLeft() on a Right value");
        }
        return left_->val();
    }
    
    R unwrapRight() const {
        if (!right_) {
            throw std::runtime_error("called unwrapRight() on a Left value");
        }
        return right_->val();
    }

private:
    types::Left<L>* left_;
    types::Right<R>* right_;
};
