set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(echo_server echo_server.cc)
add_executable(pingpong_server pingpong_server.cc)

target_link_libraries(echo_server mynetlib pthread)
target_link_libraries(pingpong_server mynetlib pthread)