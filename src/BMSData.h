#pragma once

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifdef __cplusplus
extern "C" {
#endif

PACK(struct JKBMSWarning {
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
});

PACK(struct JKBMSStatus {
	bool charging_on : 1;
	bool discharging_on : 1;
	bool balancing_on : 1;
	bool battery_connected : 1;
	unsigned _padding : 12;
});

struct JKBMSData {
	static constexpr const unsigned cell_count = 20;

	unsigned short voltage_cells_mv[cell_count];
	unsigned voltage_mv;
	int current_ma;
	char temp_mosfet;
	char temp_battery1;
	char temp_battery2;
	unsigned char soc;
	unsigned short cycle_count;
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

#ifdef __cplusplus
}
#endif
