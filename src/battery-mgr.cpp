#include <iostream>
#include <thread>
#include "battery.hpp"
#include "BMSData.h"
#include "BMSParser.hpp"
#include <influxdb.hpp>

static constexpr const char* serial_device = "/dev/serial2";

static constexpr const char* influxdb_org_name = "Autonergy";
static constexpr const char* influxdb_bucket = "Battery";

static constexpr const unsigned startCharging_mv = 20 * 3600, stopCharging_mv = 20 * 4050;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		auto influxdb_token = getenv("INFLUXDB_TOKEN");
		if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		//Battery bat(serial_device);
		influxdb_cpp::server_info serverInfo("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket);
	
		while(true) {
			try {
				/*JKBMSData data = bat.ReadAll();

				if(data.status.charging_on && data.voltage_mv > stopCharging_mv) bat.SetChargeState(false);
				else if(!data.status.charging_on && data.voltage_mv < startCharging_mv) bat.SetChargeState(true);*/

				const uint8_t buf[] = {0x4e, 0x57, 0x01, 0x0b, 0, 0, 0, 0, 6, 0, 1,
					0x79, 0x12, 1, 0x0e, 0xf9, 2, 0x0e, 0xf9, 3, 0x0e, 0xf9, 4, 0x0e, 0xf9, 5, 0x0e, 0xf9, 6, 0x0e, 0xf9,
					0x80, 0x00, 0x1b, 0x81, 0, 0x1e, 0x82, 0, 0x1e, 0x83, 0x1d, 0xbc,
					0x84, 0x27, 0x10, 0x85, 0x47, 0x86, 2, 0x87, 0, 1, 0x89, 0, 0, 0, 0, 0x8a, 0, 0x14, 0x8b, 0, 0,
					0x8c, 0, 0x0b, 0x8e, 0x20, 0xd0, 0x8f, 0x15, 0xe0};
				JKBMSData data = parseBMSData(buf);

				if(*(int16_t*)(&data.warnings) != 0) {	// There are warnings!
					influxdb_cpp::builder()
						.meas("BMSWarning")
						.tag("battery", "HouseMain")
						.field("cell-inbalance", data.warnings.cell_inbalance)
						.field("low-capacity", data.warnings.low_capacity)
						.field("undertemp", data.warnings.lowtemp)
						.field("overcurrent-charge", data.warnings.overcurrent_charge)
						.field("overcurrent-discharge", data.warnings.overcurrent_discharge)
						.field("overtemp", data.warnings.overtemp)
						.field("overtemp-box", data.warnings.overtemp_box)
						.field("overtemp-mosfet", data.warnings.overtemp_mosfet)
						.field("overvoltage", data.warnings.overvoltage)
						.field("overvoltage-cell", data.warnings.overvoltage_cell)
						.field("undervoltage", data.warnings.undervoltage)
						.field("undervoltage-cell", data.warnings.undervoltage_cell)
						.post_http(serverInfo);
				}

				influxdb_cpp::builder()
					.meas("BMSDatapoint")
					.tag("battery", "HouseMain")
					.field("voltage", data.voltage_mv / 1000.f)
					.field("current", data.current_ma / 1000.f)
					.field("cell1", data.voltage_cells_mv[0] / 1000.f)
					.field("cell2", data.voltage_cells_mv[1] / 1000.f)
					.field("cell3", data.voltage_cells_mv[2] / 1000.f)
					.field("cell4", data.voltage_cells_mv[3] / 1000.f)
					.field("cell5", data.voltage_cells_mv[4] / 1000.f)
					.field("cell6", data.voltage_cells_mv[5] / 1000.f)
					.field("cell7", data.voltage_cells_mv[6] / 1000.f)
					.field("cell8", data.voltage_cells_mv[7] / 1000.f)
					.field("cell9", data.voltage_cells_mv[8] / 1000.f)
					.field("cell10", data.voltage_cells_mv[9] / 1000.f)
					.field("cell11", data.voltage_cells_mv[10] / 1000.f)
					.field("cell12", data.voltage_cells_mv[11] / 1000.f)
					.field("cell13", data.voltage_cells_mv[12] / 1000.f)
					.field("cell14", data.voltage_cells_mv[13] / 1000.f)
					.field("cell15", data.voltage_cells_mv[14] / 1000.f)
					.field("cell16", data.voltage_cells_mv[15] / 1000.f)
					.field("cell17", data.voltage_cells_mv[16] / 1000.f)
					.field("cell18", data.voltage_cells_mv[17] / 1000.f)
					.field("cell19", data.voltage_cells_mv[18] / 1000.f)
					.field("cell20", data.voltage_cells_mv[19] / 1000.f)
					.field("temp-mosfet", data.temp_mosfet)
					.field("temp-battery1", data.temp_battery1)
					.field("temp-battery2", data.temp_battery2)
					.field("soc", data.soc)
					.field("cycle-count", data.cycle_count)
					.field("cycle-capacity", (int)data.cycle_capacity)
					.field("charge-on", data.status.charging_on)
					.field("discharge-on", data.status.discharging_on)
					.field("balance-on", data.status.balancing_on)
					.post_http(serverInfo);
			} catch(const std::exception& e) {
				std::cerr << e.what() << std::endl;
			}

			std::this_thread::sleep_for(minutes(1));
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
