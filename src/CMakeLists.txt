set(SOURCE_FILES
	Serial.hpp
	Serial.cpp

	MQTTComms.hpp
	MQTTComms.cpp

	Battery.hpp
	Battery.cpp

	battery-mgr.cpp
)
add_executable(battery-mgr ${SOURCE_FILES})

target_link_libraries(battery-mgr PUBLIC wiringPi crypt yaml-cpp PahoMqttCpp::paho-mqttpp3 flatbuffers_target)

install(TARGETS battery-mgr RUNTIME)
