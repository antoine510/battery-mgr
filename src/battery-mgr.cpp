#include <iostream>
#include <thread>
#include "battery.hpp"
#include "BMSData.h"
#include "ZMQServer.hpp"
#include <czmq.h>

static constexpr const char* serial_device = "/dev/serial2";

static constexpr const unsigned short zmq_port = 14250;
static constexpr const char* zmq_topic = "battery";

static constexpr const unsigned startCharging_mv = 20 * 3600, stopCharging_mv = 20 * 4050;

int main(int argc, char** argv) {
	using namespace std::chrono;

	try {
		Battery bat(serial_device);
		ZMQServer server(zmq_port, zmq_topic);
	
		while(true) {
			try {
				JKBMSData data = bat.ReadAll();

				if(data.status.charging_on && data.voltage_mv > stopCharging_mv) bat.SetChargeState(false);
				else if(!data.status.charging_on && data.voltage_mv < startCharging_mv) bat.SetChargeState(true);

				server.SendData(data);
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
