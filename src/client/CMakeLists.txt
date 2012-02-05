set(srcSources widget.cpp renderer.cpp)

foreach(srcSource ${srcSources})
	set(clientSources ${clientSources} ${CMAKE_CURRENT_SOURCE_DIR}/${srcSource} CACHE STRING INTERNAL FORCE)
endforeach(srcSource)

add_subdirectory(widgets)

add_executable(client main.cpp ${clientSources})
set_target_properties(client PROPERTIES OUTPUT_NAME ${executableName})
target_link_libraries(client ${executableName} ${clientLinkLibs})
