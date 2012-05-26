add_executable(master-server main.cpp)
set_target_properties(master-server PROPERTIES OUTPUT_NAME ${executableName}-master-server)
target_link_libraries(master-server ${executableName})
