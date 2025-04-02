#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

class Serial {
public:
	~Serial() noexcept;

	static void SetupInstance(const std::string& path, int baudrate = 9600) {
		inst = std::unique_ptr<Serial>(new Serial(path, baudrate));
	}
	static Serial& Instance() { return *inst; }

    class ReadTimeoutException : public std::runtime_error {
	public:
		ReadTimeoutException() : std::runtime_error("Read timed-out") {}
	};

    class WriteException : public std::runtime_error {
	public:
		WriteException() : std::runtime_error("Write failure") {}
	};

	std::vector<uint8_t> Read() const;
	void Write(const uint8_t* buf, size_t sz) const;

private:
	Serial(const std::string& path, int baudrate);

	void waitForData() const;

	inline static std::unique_ptr<Serial> inst;

	int _fd = -1;
	struct timespec _timeout = { 0, 500'000'000 };
    
    static constexpr size_t read_buf_sz = 1024;
    mutable unsigned char _read_buf[read_buf_sz];
};
