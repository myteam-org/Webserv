#pragma once

#include <cstddef>
#include "option.hpp"

class FileDescriptor {
	public:
		FileDescriptor(int fd);
		~FileDescriptor();
		types::Option<int> getFd() const;
		void setFd(int fd);
		static const int kInvalidFD = -1;
	private:
		int fd;
};
