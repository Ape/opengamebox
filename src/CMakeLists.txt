set(srcSources coordinates.cpp vector2.cpp color.cpp object.cpp net.cpp)

foreach(srcSource ${srcSources})
	set(commonSources ${commonSources} ${CMAKE_CURRENT_SOURCE_DIR}/${srcSource} CACHE STRING INTERNAL FORCE)
endforeach(srcSource)

add_library(${executableName} ${commonSources})
target_link_libraries(${executableName} ${commonLinkLibs})

add_subdirectory(client)
add_subdirectory(server)
