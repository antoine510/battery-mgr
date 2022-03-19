
########
# Battery manager
########

set(SOURCE_FILES
	BMSData.h

	BMSParser.hpp
	BMSParser.cpp

	battery.hpp
	battery.cpp

	ZMQServer.hpp
	ZMQServer.cpp

	battery-mgr.cpp
)
add_executable(battery-mgr ${SOURCE_FILES})

target_link_libraries(battery-mgr PkgConfig::CZMQ)

if(WINDOWS_HOST)
	set_target_properties(battery-mgr PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Install/bin/")
endif()

install(TARGETS battery-mgr RUNTIME)
