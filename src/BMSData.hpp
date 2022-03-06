#pragma once

#include <vector>

struct JKBMSWarning {
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

struct JKBMSStatus {
	bool charging_on : 1;
	bool discharging_on : 1;
	bool balancing_on : 1;
	bool battery_connected : 1;
	unsigned _padding : 12;
};

struct JKBMSData {
	JKBMSData(const uint8_t* buf);

	std::vector<unsigned> voltage_cells_mv;
	int temp_mosfet;
	int temp_balance;
	int temp_battery;
	unsigned voltage_mv;
	int current_ma;
	unsigned soc;
	unsigned cycle_count;
	unsigned cycle_capacity;

	JKBMSWarning warnings;
	JKBMSStatus status;

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
