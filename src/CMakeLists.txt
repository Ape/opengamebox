set(srcSources coordinates.cpp vector2.cpp color.cpp utils.cpp objectClassManager.cpp objectClass.cpp object.cpp net.cpp client.cpp)

foreach(srcSource ${srcSources})
	set(commonSources ${commonSources} ${CMAKE_CURRENT_SOURCE_DIR}/${srcSource})
endforeach(srcSource)

add_library(${executableName} ${commonSources})
target_link_libraries(${executableName} ${commonLinkLibs})

add_subdirectory(client)
add_subdirectory(server)
