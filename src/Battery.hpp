#pragma once

#include "Serial.hpp"
#include <netinet/in.h>

class Battery {
public:
	static constexpr unsigned cellCount = 20;

	Battery(const std::string& path) : _serial(path, 115200) {}

	void SetChargeState(bool enable) const;
	void SetDishargeState(bool enable) const;
	void SetCellOvervoltage(uint16_t maxVoltage_mV, uint16_t recoveryVoltage_mV) const;

	struct __attribute__((__packed__)) BMSWarning {
		bool low_capacity : 1;
		bool overtemp_mosfet : 1;
		bool overvoltage : 1;
		bool undervoltage : 1;
		bool overtemp : 1;
		bool overcurrent_charge : 1;
		bool overcurrent_discharge : 1;
		bool cell_inbalance : 1;
		bool overtemp_box : 1;
		bool lowtemp : 1;
		bool overvoltage_cell : 1;
		bool undervoltage_cell : 1;
		unsigned _padding : 4;
	};

	struct __attribute__((__packed__)) BMSStatus {
		bool charging_on : 1;
		bool discharging_on : 1;
		bool balancing_on : 1;
		bool battery_connected : 1;
		unsigned _padding : 12;
	};

	struct BMSData {
		unsigned short voltage_cells_mv[cellCount];
		unsigned short voltage_cv;
		short current_ca;
		char temp_mosfet;
		char temp_battery1;
		char temp_battery2;
		unsigned char soc;
		unsigned short cycle_count;
		unsigned cycle_capacity;
		BMSWarning warnings;
		BMSStatus status;

		/*unsigned overvolt_total;
		unsigned undervolt_total;

		unsigned overvolt_cell_trigger;
		unsigned overvolt_cell_release;
		unsigned overvolt_cell_delay;

		unsigned undervolt_cell_trigger;
		unsigned undervolt_cell_release;
		unsigned undervolt_cell_delay;

		unsigned inbalance_max;

		unsigned current_discharge_max;
		unsigned current_discharge_delay;

		unsigned current_charge_max;
		unsigned current_charge_delay;

		unsigned balance_start_mv;
		unsigned balance_min_mv_diff;
		bool balance_on;

		unsigned mosfet_temp_trigger;
		unsigned mosfet_temp_release;
		unsigned balance_temp_trigger;
		unsigned balance_temp_release;

		unsigned temp_max_diff;
		unsigned temp_charge_trigger;
		unsigned temp_discharge_trigger;
		unsigned temp_charge_low_trigger;
		unsigned temp_charge_low_release;
		unsigned temp_discharge_low_trigger;
		unsigned temp_discharge_low_release;

		unsigned poweron_minutes;*/
	};

	BMSData ReadAll() const;
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

	struct __attribute__((__packed__)) RdBuf {
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
	};
	struct __attribute__((__packed__)) CmdBuf {
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
	};
	struct __attribute__((__packed__)) Cmd16Buf {
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

	BMSData parseBMSData(const std::vector<uint8_t>& buf) const;

	Serial _serial;
};
