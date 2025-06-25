#pragma once

#include <cstddef> // for std::size_t

namespace io {

	typedef Result<std::size_t, error::AppError> ReadResult;

	class IReader {
	public:
		virtual ~IReader() {}
		virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
		virtual bool eof() = 0;
	};

} // namespace io
