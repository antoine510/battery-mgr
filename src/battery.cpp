#include "battery.hpp"
#include "BMSParser.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>

JKBMSData Battery::ReadAll() {
	RdBuf buf;
	buf.cmd = CmdWord::READ_ALL;
	buf.data_identification = 0;
	_serial.Write(finalizeBuf(buf), sizeof(buf));
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return parseBMSData(_serial.Read().data());
}

void Battery::SetChargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = 0xab;
	buf.write_byte = enable;
	_serial.Write(finalizeBuf(buf), sizeof(buf));
	_serial.Read();
}

void Battery::SetDishargeState(bool enable) const {
	CmdBuf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = 0xac;
	buf.write_byte = enable;
	_serial.Write(finalizeBuf(buf), sizeof(buf));
	_serial.Read();
}

void Battery::SetCellOvervoltage(uint16_t maxVoltage_mV, uint16_t recoveryVoltage_mV) const {
	Cmd16Buf buf;
	buf.cmd = CmdWord::WRITE;
	buf.data_identification = Register::CELL_OVERVOLTAGE_CUT;
	buf.write_word = htons(maxVoltage_mV);
	_serial.Write(finalizeBuf(buf), sizeof(buf));
	_serial.Read();

	buf.data_identification = Register::CELL_OVERVOLTAGE_RECOVER;
	buf.write_word = htons(recoveryVoltage_mV);
	_serial.Write(finalizeBuf(buf), sizeof(buf));
	_serial.Read();
}
