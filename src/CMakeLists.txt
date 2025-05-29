
########
# Battery manager
########

set(SOURCE_FILES
	BMSData.h

	BMSParser.hpp
	BMSParser.cpp

	Serial.hpp
	Serial.cpp
	MQTTComms.hpp
	MQTTComms.cpp

	battery.hpp
	battery.cpp
)
add_executable(battery-mgr ${SOURCE_FILES} battery-mgr.cpp)
target_link_libraries(battery-mgr wiringPi pthread crypt PahoMqttCpp::paho-mqttpp3 flatbuffers_target)

install(TARGETS battery-mgr RUNTIME)
