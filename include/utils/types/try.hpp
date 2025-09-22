#ifndef UTILS_TYPES_TRY_HPP
#define UTILS_TYPES_TRY_HPP

#include "utils/types/result.hpp"
#include "utils/types/option.hpp"

template <typename T, typename E>
Err<E> tryDefault(const types::Result<T, E> &res) {
	return types::err(res.unwrapErr());
}

template <typename T>
types::Option<T> tryDefault(const types::Option<T> &opt) {
	(void)opt;
    return types::Option<T>(types::none<T>());
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

// C++でRust風のTRYマクロを実現するための仕組み
// 失敗を安全に早期リターンしつつ、成功時は値を取り出す
// ＜2つのオーバーロード関数＞TRY内部で使われるErrやNoneを返す補助関数
//  tryDefault(Result<T, E>):失敗時にErr<E>をそのまま返す
//  tryDefault(Option<T>):値なし（None）の場合にOption<T>(None)をそのまま返す
// ＜TRY(expr)マクロ＞
//  exprを一度だけ評価する
//  canUnwrap()（=成功かどうか）をチェック
//  失敗していればreturn
//  成功していれば.unwrap()で値を取り出す
// 安全・簡潔・多重評価なし・関数型風のエラーハンドリンクが可能
