#include "BMSData.hpp"

#include <stdexcept>
#include <string>

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#define FIELD(x) if(*p++ != x) throw std::runtime_error("Missing field " + std::to_string(x));
#define U16(x) ntohs(*(const uint16_t*)(x))
#define U32(x) ntohl(*(const uint32_t*)(x))
#define TEMP(x) U16(x) > 100 ? 100 - U16(x) : U16(x)

JKBMSData::JKBMSData(const uint8_t* buf) {
	auto p = buf + 11;  // Skip 4e 57 xx xx 00 00 00 00 06 00 01
	uint16_t len = U16(buf + 2) + 2;
	FIELD(0x79)
	unsigned cellCount = *p++ / 3;
	voltage_cells_mv.resize(cellCount);
	for(unsigned i = 0; i < cellCount; ++i, p += 3) {
		voltage_cells_mv[i] = U16(p + 1);
	}
	FIELD(0x80)
	temp_mosfet = TEMP(p);
	p += 2;
	FIELD(0x81)
	temp_balance = TEMP(p);
	p += 2;
	FIELD(0x82)
	temp_battery = TEMP(p);
	p += 2;
	FIELD(0x83)
	voltage_mv = U16(p) * 10;
	p += 2;
	FIELD(0x84)
	auto unsigned_current = U16(p);
	current_ma = ((unsigned_current & 0x8000) ? ~unsigned_current + 1 : unsigned_current) * 10;
	p += 2;
	FIELD(0x85)
	soc = *p++;
	FIELD(0x86)
	++p;
	FIELD(0x87)
	cycle_count = U16(p);
	p += 2;
	FIELD(0x89)
	cycle_capacity = U32(p);
	p += 4;
	FIELD(0x8a)
	p += 2;
	FIELD(0x8b)
	*(uint16_t*)(&warnings) = U16(p);
	p += 2;
	FIELD(0x8c)
	*(uint16_t*)(&status) = U16(p);
	p += 2;
}
