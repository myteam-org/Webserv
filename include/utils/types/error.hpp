#pragma once

namespace error {
     enum AppError {
        kIOUnknown,
        kBadRequest,
        kRequestEntityTooLarge,   // ← 追加：413 Payload Too Large
        kBadMethod,
        kBadHttpVersion,
        kMissingHost,
        kInvalidContentLength,
        kInvalidTransferEncoding,
        kHasContentLengthAndTransferEncoding,
        kcontainsNonDigit,
        kBadLocationContext,
        kUriTooLong
    };
    enum SystemError {
        kUnknownError,
        // ソケット／fd 関連
        kSocketCreateFailed,
        kBindFailed,
        kListenFailed,
        kAcceptFailed,
        kConnectFailed,
        kSetNonBlockingFailed,
        kCloseFailed,

        // IO 関連
        kReadFailed,
        kWriteFailed,
        kPipeCreateFailed,
        kDup2Failed,
        kWaitPidFailed,

        // epoll/kqueue 関連
        kEpollCreateFailed,
        kEpollCtlFailed,
        kEpollWaitFailed,
        kKqueueCreateFailed,
        kKeventCtlFailed,
        kKeventWaitFailed,

        // ファイル操作関連
        kOpenFailed,
        kStatFailed,
        kAccessDenied,
        kDirOpenFailed,
        kDirReadFailed,

        kForkFailed
    };
} // namespace error
