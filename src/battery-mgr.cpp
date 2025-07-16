#include <iostream>
#include <thread>
#include <condition_variable>
#include <signal.h>

#include "Battery.hpp"
#include <influxdb.hpp>
#include <yaml-cpp/yaml.h>
#include <wiringPi.h>

#include "MQTTComms.hpp"
#include "battery_generated.h"

constexpr int heater_on_temp = 4;
constexpr unsigned short heater_min_voltage_cv = 6600;

using SamplePeriod = std::chrono::duration<int64_t, std::ratio<5>>;
using LogPeriod = std::chrono::minutes;
using ExtraLogPeriod = std::chrono::hours;	// Needs to be a multiple of LogPeriod

bool serviceRunning = true;
std::mutex serviceMutex;
std::condition_variable serviceCV;
void signalHandler(int signum) {
	std::cout << "Closing service" << std::endl;
	serviceRunning = false;
	serviceCV.notify_all();
}

int main(int argc, char** argv) {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	YAML::Node config = YAML::LoadFile("config.yaml");
	const auto influxConfig = config["influx"];
	const auto batteryConfig = config["battery"];
	const auto heaterConfig = config["heater"];

	MQTTComms mqtt("battery-mgr");

	int heaterPin = heaterConfig["pin"].as<int>();
	wiringPiSetup();
	pinMode(heaterPin, OUTPUT);
	digitalWrite(heaterPin, LOW);
	bool heater_on = digitalRead(heaterPin);

	Battery bat(batteryConfig["dev"].as<std::string>());
	influxdb_cpp::server_info serverInfo(influxConfig["ip"].as<std::string>(), 8086, influxConfig["org"].as<std::string>(), influxConfig["token"].as<std::string>(), influxConfig["bucket"].as<std::string>());

	auto now = std::chrono::system_clock::now();
	auto nextLogTP = std::chrono::ceil<LogPeriod>(now);
	auto nextExtraTP = std::chrono::ceil<ExtraLogPeriod>(now);

	std::unique_lock lk(serviceMutex);
	while(serviceRunning) {
		now = std::chrono::system_clock::now();
		if(serviceCV.wait_until(lk, std::chrono::ceil<SamplePeriod>(now)) != std::cv_status::timeout) continue;

		try {
			Battery::BMSData data = bat.ReadAll();

			/****** HEATER MANAGEMENT ******/
			auto avgTemp = (data.temp_battery1 + data.temp_battery2) / 2;
			if(!heater_on && avgTemp <= heaterConfig["onTemp"].as<int>() && data.voltage_cv > heaterConfig["minVoltage_cv"].as<int>()) {
				digitalWrite(heaterPin, HIGH);
				heater_on = true;
			} else if(heater_on && avgTemp >= heaterConfig["offTemp"].as<int>()) {
				digitalWrite(heaterPin, LOW);
				heater_on = false;
			}

			/*** MQTT publishing ***/
			{
				flatbuffers::FlatBufferBuilder builder;
				auto cellVoltages = builder.CreateVector(data.voltage_cells_mv, 20);
				api::BatteryState state = api::BatteryState::OK;
				if(data.warnings.overcurrent_charge || data.warnings.overcurrent_discharge) state = api::BatteryState::Overcurrent;
				else if(data.warnings.overtemp || data.warnings.overtemp_box || data.warnings.overtemp_mosfet) state = api::BatteryState::Overtemperature;
				else if(data.warnings.overvoltage || data.warnings.overvoltage_cell) state = api::BatteryState::Overvoltage;
				else if(data.warnings.lowtemp) state = api::BatteryState::Undertemperature;
				else if(data.warnings.undervoltage || data.warnings.undervoltage_cell) state = api::BatteryState::Undervoltage;
				auto batFlat = api::CreateBattery(builder, data.soc, data.voltage_cv, data.current_ca, cellVoltages, (data.temp_battery1 + data.temp_battery2) / 2, state);
				builder.Finish(batFlat);
				mqtt.Publish("Battery/House", builder);
			}
			

			if(now >= nextLogTP) {

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

				influxdb_cpp::builder builder;

				builder.meas("BMSDatapoint")
					.field("voltage", data.voltage_cv / 100.f, 2)
					.field("current", data.current_ca / 100.f, 2)
					.field("temp-mosfet", data.temp_mosfet)
					.field("temp-battery1", data.temp_battery1)
					.field("temp-battery2", data.temp_battery2)
					.field("charge-on", data.status.charging_on)
					.field("discharge-on", data.status.discharging_on)
					.field("balance-on", data.status.balancing_on)
					.field("heater-on", heater_on);

				if(now >= nextExtraTP) {
					((influxdb_cpp::detail::field_caller&)builder)
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
						.field("cycle-count", data.cycle_count)
						.field("cycle-capacity", (int)data.cycle_capacity);

					nextExtraTP = std::chrono::ceil<ExtraLogPeriod>(now);
				}
				((influxdb_cpp::detail::field_caller&)builder).post_http(serverInfo);

				nextLogTP = std::chrono::ceil<LogPeriod>(now);
			}
		} catch(const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
	}

	return 0;
}
