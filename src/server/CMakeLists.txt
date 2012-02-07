add_executable(server main.cpp)
set_target_properties(server PROPERTIES OUTPUT_NAME ${executableName}-server)
target_link_libraries(server ${executableName})
