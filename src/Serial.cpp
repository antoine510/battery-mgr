#include "Serial.hpp"

#include <cstring>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>

#define GET_ERROR_STR std::string(strerror(errno))
#define GET_ERROR errno

Serial::Serial(const std::string& path, int baudrate) {
	if (_fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC); _fd < 0)
		throw std::runtime_error("Open failed: " + GET_ERROR_STR);

	termios2 tc{};

	tc.c_iflag = 0;
	tc.c_oflag = 0;
	tc.c_lflag = 0;
	tc.c_cflag = CLOCAL | CS8 | BOTHER;
	tc.c_cc[VMIN] = 0;	// Immediate read
	tc.c_cc[VTIME] = 0;
	tc.c_ispeed = baudrate;
	tc.c_ospeed = baudrate;

	if(ioctl(_fd, TCSETS2, &tc) != 0) {
		::close(_fd);
		throw std::runtime_error("TCSETS2 failed: " + GET_ERROR_STR);
	}
}

Serial::~Serial() noexcept {
	if (_fd > -1) ::close(_fd);
	_fd = -1;
}

void Serial::waitForData() const {
	fd_set rx_fd_set{};
	FD_SET(_fd, &rx_fd_set);

	int res = pselect(_fd + 1, &rx_fd_set, nullptr, nullptr, &_timeout, nullptr);
	if(res < 0) {
		throw std::runtime_error("Select failed: " + GET_ERROR_STR);
	} else if(res == 0) {
		throw ReadTimeoutException();
	}
}

std::vector<uint8_t> Serial::Read() const {
	waitForData();

	std::this_thread::sleep_for(std::chrono::milliseconds(500));	// Wait for all data

	int read_len = ::read(_fd, _read_buf, read_buf_sz);
	if (read_len <= 0) throw ReadTimeoutException();

	auto ret = std::vector<uint8_t>(read_len);
	std::memcpy(ret.data(), _read_buf, read_len);
	return ret;
}

void Serial::Write(const uint8_t* buf, size_t sz) const {
	ioctl(_fd, TCFLSH, 2);	// Flush read and write kernel queues
	if (::write(_fd, buf, sz) != sz) throw WriteException();
}
