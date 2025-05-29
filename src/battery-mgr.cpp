#include <iostream>
#include <thread>
#include "battery.hpp"
#include "BMSData.h"
#include "BMSParser.hpp"
#include "MQTTComms.hpp"
#include "battery_generated.h"
#include <influxdb.hpp>
#include <wiringPi.h>

constexpr int heater_pin = 16;
constexpr int heater_on_temp = 4;
constexpr unsigned short heater_min_voltage_cv = 6600;

constexpr const char* serial_device = "/dev/battery";

constexpr const char* influxdb_org_name = "Microtonome";
constexpr const char* influxdb_bucket = "Batterie";

using LogPeriod = std::chrono::duration<int64_t, std::ratio<300>>;

int main(int argc, char** argv) {
	using namespace std::chrono;

	wiringPiSetup();
	pinMode(heater_pin, OUTPUT);
	digitalWrite(heater_pin, LOW);
	bool heater_on = false;

	Serial::SetupInstance(serial_device, 115200);

	MQTTComms mqtt("battery-mgr");

	try {
		auto influxdb_token = getenv("INFLUXDB_TOKEN");
		if(!influxdb_token) throw std::invalid_argument("Missing INFLUXDB_TOKEN environment variable");

		Battery bat;
		influxdb_cpp::server_info serverInfo("127.0.0.1", 8086, influxdb_org_name, influxdb_token, influxdb_bucket);

		while(true) {
			const auto currentTP = std::chrono::system_clock::now();
			const auto nextTP = std::chrono::ceil<LogPeriod>(currentTP);
			std::this_thread::sleep_until(nextTP);

			try {
				JKBMSData data = bat.ReadAll();

				/****** HEATER MANAGEMENT ******/
				if((data.temp_battery1 + data.temp_battery2) / 2 <= heater_on_temp && data.voltage_cv > heater_min_voltage_cv) {
					digitalWrite(heater_pin, HIGH);
					heater_on = true;
				} else {
					digitalWrite(heater_pin, LOW);
					heater_on = false;
				}

				if(*(int16_t*)(&data.warnings) != 0) {	// There are warnings!
					influxdb_cpp::builder()
						.meas("BMSWarning")
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
					.field("voltage", data.voltage_cv / 100.f, 2)
					.field("current", data.current_ca / 100.f, 2)
					.field("cell1", data.voltage_cells_mv[0] / 1000.f, 3)
					.field("cell2", data.voltage_cells_mv[1] / 1000.f, 3)
					.field("cell3", data.voltage_cells_mv[2] / 1000.f, 3)
					.field("cell4", data.voltage_cells_mv[3] / 1000.f, 3)
					.field("cell5", data.voltage_cells_mv[4] / 1000.f, 3)
					.field("cell6", data.voltage_cells_mv[5] / 1000.f, 3)
					.field("cell7", data.voltage_cells_mv[6] / 1000.f, 3)
					.field("cell8", data.voltage_cells_mv[7] / 1000.f, 3)
					.field("cell9", data.voltage_cells_mv[8] / 1000.f, 3)
					.field("cell10", data.voltage_cells_mv[9] / 1000.f, 3)
					.field("cell11", data.voltage_cells_mv[10] / 1000.f, 3)
					.field("cell12", data.voltage_cells_mv[11] / 1000.f, 3)
					.field("cell13", data.voltage_cells_mv[12] / 1000.f, 3)
					.field("cell14", data.voltage_cells_mv[13] / 1000.f, 3)
					.field("cell15", data.voltage_cells_mv[14] / 1000.f, 3)
					.field("cell16", data.voltage_cells_mv[15] / 1000.f, 3)
					.field("cell17", data.voltage_cells_mv[16] / 1000.f, 3)
					.field("cell18", data.voltage_cells_mv[17] / 1000.f, 3)
					.field("cell19", data.voltage_cells_mv[18] / 1000.f, 3)
					.field("cell20", data.voltage_cells_mv[19] / 1000.f, 3)
					.field("temp-mosfet", data.temp_mosfet)
					.field("temp-battery1", data.temp_battery1)
					.field("temp-battery2", data.temp_battery2)
					.field("soc", data.soc)
					.field("cycle-count", data.cycle_count)
					.field("cycle-capacity", (int)data.cycle_capacity)
					.field("charge-on", data.status.charging_on)
					.field("discharge-on", data.status.discharging_on)
					.field("balance-on", data.status.balancing_on)
					.field("heater-on", heater_on)
					.post_http(serverInfo);

				flatbuffers::FlatBufferBuilder builder;
				auto cellVoltages = builder.CreateVector(data.voltage_cells_mv, 20);
				api::BatteryState state = api::BatteryState::OK;
				if(data.warnings.overcurrent_charge || data.warnings.overcurrent_discharge) state = api::BatteryState::Overcurrent;
				else if(data.warnings.overtemp || data.warnings.overtemp_box || data.warnings.overtemp_mosfet) state = api::BatteryState::Overtemperature;
				else if(data.warnings.overvoltage || data.warnings.overvoltage_cell) state = api::BatteryState::Overvoltage;
				else if(data.warnings.lowtemp) state = api::BatteryState::Undertemperature;
				else if(data.warnings.undervoltage || data.warnings.undervoltage_cell) state = api::BatteryState::Undervoltage;
				api::CreateBattery(builder, data.soc, data.voltage_cv, data.current_ca, cellVoltages, (data.temp_battery1 + data.temp_battery2) / 2, state);
				mqtt.Publish("Battery/House", builder);
			} catch(const std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
