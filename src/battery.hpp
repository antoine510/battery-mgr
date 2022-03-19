#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#ifndef WINDOWS
#include <netinet/in.h>
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif


class Battery {
public:
	Battery(const std::string& path, int baudrate = 115200);
	~Battery() noexcept;

	struct JKBMSData ReadAll();

	void SetChargeState(bool enable) const;
	void SetDishargeState(bool enable) const;
private:
	enum CmdWord : uint8_t {
		ACTIVATE = 1,
		WRITE = 2,
		READ = 3,
		PASSWD = 5,
		READ_ALL = 6
	};

	PACK(struct RdBuf {
		uint16_t stx = 0x574e;
		uint16_t len = 0;
		uint32_t bms_id = 0;
		uint8_t cmd = CmdWord::READ_ALL;
		uint8_t src = 3;    // Computer
		uint8_t type = 0;
		uint8_t data_identification = 0;
		uint32_t record_number = 0;
		uint8_t end_identity = 0x68;
		uint32_t cksum = 0;
	});
	PACK(struct CmdBuf {
		uint16_t stx = 0x574e;
		uint16_t len = 0;
		uint32_t bms_id = 0;
		uint8_t cmd = CmdWord::WRITE;
		uint8_t src = 3;    // Computer
		uint8_t type = 0;
		uint8_t data_identification = 0;
		uint8_t write_byte = 0;
		uint32_t record_number = 0;
		uint8_t end_identity = 0x68;
		uint32_t cksum = 0;
	});

	class ReadTimeoutException : public std::runtime_error {
	public:
		ReadTimeoutException() : std::runtime_error("Read timed-out") {}
	};

	template<typename T>
	uint8_t* finalizeBuf(T& buf) const {
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&buf);
		buf.len = htons(sizeof(T) - sizeof(T::stx));
		buf.cksum = 0;
		for (unsigned i = 0; i < sizeof(T) - sizeof(T::cksum); ++i) {
			buf.cksum += bytes[i];
		}
		buf.cksum = htonl(buf.cksum);
		return bytes;
	}

	std::vector<uint8_t> read() const;
	void write(const uint8_t* cmd_buf, size_t sz) const;

#ifndef WINDOWS
	int _fd = -1;
	struct timespec _timeout = { 0, 100000000 };	// 100 ms
#else
	void* _handle = nullptr;
#endif
};
