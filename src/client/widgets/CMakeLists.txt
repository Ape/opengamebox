set(widgets inputBox.cpp progressBar.cpp)

foreach(widget ${widgets})
	set(clientSources ${clientSources} ${CMAKE_CURRENT_SOURCE_DIR}/${widget} CACHE STRING INTERNAL FORCE)
endforeach(widget)
