
########
# Battery manager
########

set(SOURCE_FILES
	BMSData.h

	BMSParser.hpp
	BMSParser.cpp

	SerialHandler.hpp
	SerialHandler.cpp

	battery.hpp
	battery.cpp
)
add_executable(battery-mgr ${SOURCE_FILES} battery-mgr.cpp)
target_link_libraries(battery-mgr wiringPi pthread crypt)

########
# Battery programmer
########

add_executable(battery-prgm ${SOURCE_FILES} battery-prgm.cpp)


install(TARGETS battery-mgr RUNTIME)
install(TARGETS battery-prgm RUNTIME)
