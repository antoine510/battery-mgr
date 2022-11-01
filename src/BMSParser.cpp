#include "BMSParser.hpp"


#include <stdexcept>
#include <string>
#include <iostream>

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#define FIELD(x) if(*p++ != x) throw std::runtime_error("Missing field " + std::to_string(x));
#define U16(x) ntohs(*(const uint16_t*)(x))
#define U32(x) ntohl(*(const uint32_t*)(x))
#define TEMP(x) (char)(U16(x) > 100 ? 100 - U16(x) : U16(x))

JKBMSData parseBMSData(const unsigned char* buf) {
	JKBMSData res{};
	auto p = buf + 11;  // Skip 4e 57 xx xx 00 00 00 00 06 00 01
	uint16_t len = U16(buf + 2) + 2;
	FIELD(0x79)
	unsigned cellCount = *p++ / 3;
	if(cellCount != JKBMSData::cell_count) std::cerr << "Wrong cell count\n";
	for(unsigned i = 0; i < cellCount; ++i, p += 3) {
		if(i < JKBMSData::cell_count) res.voltage_cells_mv[i] = U16(p + 1);
	}
	FIELD(0x80)
	res.temp_mosfet = TEMP(p);
	p += 2;
	FIELD(0x81)
	res.temp_battery1 = TEMP(p);
	p += 2;
	FIELD(0x82)
	res.temp_battery2 = TEMP(p);
	p += 2;
	FIELD(0x83)
	res.voltage_mv = U16(p) * 10;
	p += 2;
	FIELD(0x84)
	auto unsigned_current = U16(p);
	res.current_ma = ((unsigned_current & 0x8000) ? ~unsigned_current + 1 : unsigned_current) * 10;
	p += 2;
	FIELD(0x85)
	res.soc = *p++;
	FIELD(0x86)
	++p;
	FIELD(0x87)
	res.cycle_count = U16(p);
	p += 2;
	FIELD(0x89)
	res.cycle_capacity = U32(p);
	p += 4;
	FIELD(0x8a)
	p += 2;
	FIELD(0x8b)
	*(uint16_t*)(&res.warnings) = U16(p);
	p += 2;
	FIELD(0x8c)
	*(uint16_t*)(&res.status) = U16(p);
	p += 2;
    return res;
}

