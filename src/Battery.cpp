#include "Battery.hpp"
#include <iostream>

Battery::BMSData Battery::ReadAll() const {
	RdBuf rdbuf;
	return parseBMSData(_serial.SendCommandResponse(finalizeBuf(rdbuf), sizeof(rdbuf)));
}

void Battery::SetChargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = CHARGE_MOS_ENABLE;
	buf.write_byte = enable;
	_serial.SendCommand(finalizeBuf(buf), sizeof(buf));
}

void Battery::SetDishargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = DISCHARGE_MOS_ENABLE;
	buf.write_byte = enable;
	_serial.SendCommandResponse(finalizeBuf(buf), sizeof(buf));
}

void Battery::SetCellOvervoltage(uint16_t maxVoltage_mV, uint16_t recoveryVoltage_mV) const {
	Cmd16Buf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = CELL_OVERVOLTAGE_CUT;
	buf.write_word = htons(maxVoltage_mV);
	_serial.SendCommandResponse(finalizeBuf(buf), sizeof(buf));

	buf.data_identification = CELL_OVERVOLTAGE_RECOVER;
	buf.write_word = htons(recoveryVoltage_mV);
	_serial.SendCommandResponse(finalizeBuf(buf), sizeof(buf));
}

#define FIELD(x) if(*p++ != x) throw std::runtime_error("Missing field " + std::to_string(x));
#define U16(x) ntohs(*(const uint16_t*)(x))
#define U32(x) ntohl(*(const uint32_t*)(x))
#define TEMP(x) U16(x) > 99 ? 99 - (int)U16(x) : U16(x)

Battery::BMSData Battery::parseBMSData(const std::vector<uint8_t>& buf) const {
	BMSData res{};
	auto p = buf.data() + 11;  // Skip 4e 57 xx xx 00 00 00 00 06 00 01
	uint16_t len = U16(buf.data() + 2) + 2;
	FIELD(0x79)
	if(*p++ / 3 != cellCount) throw std::runtime_error("Wrong cell count");
	for(unsigned i = 0; i < cellCount; ++i, p += 3) res.voltage_cells_mv[i] = U16(p + 1);
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
	res.voltage_cv = U16(p);
	p += 2;
	FIELD(0x84)
	auto unsigned_current = U16(p);
	res.current_ca = ((unsigned_current & 0x8000) ? -(unsigned_current & 0x7fff) : unsigned_current);
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
