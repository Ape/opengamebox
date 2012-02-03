### Common definitions ###
project(OpenGameBox)
set(executableName ogb)

cmake_minimum_required(VERSION 2.6)

add_definitions(-std=c++0x)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

IF(WIN32 AND MINGW)
	add_definitions(-DWIN32)
ENDIF(WIN32 AND MINGW)

### User selectable options ###
option(DEBUG "Show all warnings, include debug symbols, and disable optimizations" ON)

if(DEBUG)
	add_definitions(-Wall -g)
else(DEBUG)
	add_definitions(-s -O3)
endif(DEBUG)

### External libraries ###
set(commonLinkLibs enet)
set(clientLinkLibs allegro allegro_image allegro_font allegro_ttf allegro_primitives)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
link_directories(${PROJECT_BINARY_DIR}/lib)
set(BUILD_SHARED_LIBS ON)

### Internal libraries ###
add_subdirectory(src)
