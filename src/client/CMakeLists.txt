set(srcSources widget.cpp renderer.cpp)

foreach(srcSource ${srcSources})
	set(clientSources ${clientSources} ${CMAKE_CURRENT_SOURCE_DIR}/${srcSource} CACHE STRING INTERNAL FORCE)
endforeach(srcSource)

add_subdirectory(widgets)
if(MINGW AND NOT DEBUG)
	add_executable(client WIN32 main.cpp ${clientSources})
else(MINGW AND NOT DEBUG)
	add_executable(client main.cpp ${clientSources})
endif(MINGW AND NOT DEBUG)
set_target_properties(client PROPERTIES OUTPUT_NAME ${executableName})
target_link_libraries(client ${executableName} ${clientLinkLibs})

