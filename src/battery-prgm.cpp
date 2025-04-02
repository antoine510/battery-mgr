#include <iostream>
#include <thread>
#include "battery.hpp"

static constexpr const char* serial_device = "/dev/battery";

int main(int argc, char** argv) {
	try {
		Serial::SetupInstance(serial_device, 115200);

		Battery bat;
		bat.SetCellOvervoltage(4000, 3900);

		std::cout << "SetCellOvervoltage OK!" << std::endl;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
