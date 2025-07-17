#pragma once

namespace error {
     enum AppError {
        kIOUnknown,
        kRequestEntityTooLarge,   // ← 追加：413 Payload Too Large
    };
} // namespace error
