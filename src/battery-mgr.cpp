#include <iostream>
#include <thread>
#include "battery.hpp"
#include "BMSData.hpp"

static constexpr const char* serial_device = "/dev/serial2";

static constexpr const unsigned startCharging_mv = 20 * 3600, stopCharging_mv = 20 * 4050;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		Battery bat(serial_device);
	
		while(true) {
			try {
				JKBMSData data = bat.ReadAll();

				if(data.status.charging_on && data.voltage_mv > stopCharging_mv) bat.SetChargeState(false);
				else if(!data.status.charging_on && data.voltage_mv < startCharging_mv) bat.SetChargeState(true);
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
