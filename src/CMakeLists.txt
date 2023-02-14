
########
# Battery manager
########

set(SOURCE_FILES
	BMSData.h

	BMSParser.hpp
	BMSParser.cpp

	battery.hpp
	battery.cpp

	battery-mgr.cpp
)
add_executable(battery-mgr ${SOURCE_FILES})
target_link_libraries(battery-mgr wiringPi)

install(TARGETS battery-mgr RUNTIME)
