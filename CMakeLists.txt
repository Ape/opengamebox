### Common definitions ###
project(OpenGameBox CXX)
set(executableName opengamebox)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS -std=c++0x)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

if(WIN32 AND MINGW)
	add_definitions(-DWIN32)
endif(WIN32 AND MINGW)

### User selectable options ###
option(DEBUG "Show all warnings, include debug symbols, and disable optimizations" ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wold-style-cast")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")

if(DEBUG)
	set(CMAKE_BUILD_TYPE debug)
else(DEBUG)
	set(CMAKE_BUILD_TYPE release)
endif(DEBUG)

### External libraries ###
set(commonLinkLibs enet config++ physfs)
set(clientLinkLibs allegro allegro_image allegro_font allegro_ttf allegro_primitives allegro_color allegro_physfs)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
link_directories(${PROJECT_BINARY_DIR}/lib)
set(BUILD_SHARED_LIBS ON)

### Internal libraries ###
add_subdirectory(src)
