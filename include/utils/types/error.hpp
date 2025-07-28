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
        kContainNonDigit,
    };
} // namespace error
