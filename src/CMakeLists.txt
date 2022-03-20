
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

if(WINDOWS_HOST)
	set_target_properties(battery-mgr PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Install/bin/")
endif()

install(TARGETS battery-mgr RUNTIME)
