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
    };
    
    template<typename T>
    class Option {
        Some<T>* some_;
        bool is_none_;
        
        Option(const Option&);
        Option& operator=(const Option&);
        
    public:
        // NOLINTNEXTLINE(google-explicit-constructor)
        Option(types::Some<T> some) : some_(new types::Some<T>(some)), is_none_(false) {}
        
        // NOLINTNEXTLINE(google-explicit-constructor)
        Option(types::None none_val) : some_(NULL), is_none_(true) {
            (void)none_val;
        }
        
        ~Option() {
            delete some_;
        }
        
        bool is_some() const { return !is_none_; }
        bool is_none() const { return is_none_; }
        
        T unwrap() const {
            if (is_none()) {
                throw std::runtime_error("Called unwrap on None");
            }
            return some_->value();
        }
        
        T unwrap_or(const T& default_val) const {
            if (is_some()) {
                return some_->value();
            }
            return default_val;
        }
    };
    
    template<typename T>
    Some<T> some(const T& val) { return Some<T>(val); }
    
    const None none = None();
    
} // namespace types
