#include "battery.hpp"
#include "BMSParser.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>

JKBMSData Battery::ReadAll() {
	RdBuf rdbuf;
	Serial::Instance().Write(finalizeBuf(rdbuf), sizeof(RdBuf));
	return parseBMSData(Serial::Instance().Read().data());
}

void Battery::SetChargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = 0xab;
	buf.write_byte = enable;
	Serial::Instance().Write(finalizeBuf(buf), sizeof(buf));
	Serial::Instance().Read();
}

void Battery::SetDishargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = 0xac;
	buf.write_byte = enable;
	Serial::Instance().Write(finalizeBuf(buf), sizeof(buf));
	Serial::Instance().Read();
}

void Battery::SetCellOvervoltage(uint16_t maxVoltage_mV, uint16_t recoveryVoltage_mV) const {
	Cmd16Buf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = Register::CELL_OVERVOLTAGE_CUT;
	buf.write_word = htons(maxVoltage_mV);
	Serial::Instance().Write(finalizeBuf(buf), sizeof(buf));
	Serial::Instance().Read();

	buf.data_identification = Register::CELL_OVERVOLTAGE_RECOVER;
	buf.write_word = htons(recoveryVoltage_mV);
	Serial::Instance().Write(finalizeBuf(buf), sizeof(buf));
	Serial::Instance().Read();
}
