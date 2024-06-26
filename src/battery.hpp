#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "SerialHandler.hpp"
#include <netinet/in.h>

#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))


class Battery {
public:
	Battery(const std::string& path, int baudrate = 115200) : _serial(path, baudrate) {}

	struct JKBMSData ReadAll();

	void SetChargeState(bool enable) const;
	void SetDishargeState(bool enable) const;
	void SetCellOvervoltage(uint16_t maxVoltage_mV, uint16_t recoveryVoltage_mV) const;
private:
	enum CmdWord : uint8_t {
		ACTIVATE = 1,
		WRITE = 2,
		READ = 3,
		PASSWD = 5,
		READ_ALL = 6
	};

	enum Register : uint8_t {
		TOTAL_OVERVOLTAGE = 0x8e,			// [cV]
		TOTAL_UNDERVOLTAGE = 0x8f,			// [cV]
		CELL_OVERVOLTAGE_CUT = 0x90,		// [mV]
		CELL_OVERVOLTAGE_RECOVER = 0x91,	// [mV]
		CELL_UNDERVOLTAGE_CUT = 0x93,		// [mV]
		CELL_UNDERVOLTAGE_RECOVER = 0x94,	// [mV]
		CHARGE_MOS_ENABLE = 0xab,
		DISCHARGE_MOS_ENABLE = 0xac
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
	PACK(struct Cmd16Buf {
		uint16_t stx = 0x574e;
		uint16_t len = 0;
		uint32_t bms_id = 0;
		uint8_t cmd = CmdWord::WRITE;
		uint8_t src = 3;    // Computer
		uint8_t type = 0;
		uint8_t data_identification = 0;
		uint16_t write_word = 0;
		uint32_t record_number = 0;
		uint8_t end_identity = 0x68;
		uint32_t cksum = 0;
	});

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

	SerialHandler _serial;
};
