#ifndef UTILS_TYPES_TRY_HPP
#define UTILS_TYPES_TRY_HPP

#include "result.hpp"
#include "option.hpp"

template <typename T, typename E>
Err<E> tryDefault(const types::Result<T, E> &res) {
	return types::err(res.unwrapErr());
}

template <typename T>
types::Option<T> tryDefault(const types::Option<T> &opt) {
	(void)opt;
    return types::Option<T>(types::none);
}

// typeof(expr) e = (expr); の箇所で明示的に一度実行する。
// それにより、exprが複数回実行されることを防ぐ。
#define TRY(expr)                                                                \
    ({                                                                           \
        const __typeof__(expr) e = (expr); /* NOLINT(readability-identifier-length) */ \
        if (!e.canUnwrap()) return tryDefault(e);                                \
        e.unwrap();                                                              \
    })


#endif

