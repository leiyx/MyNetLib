add_executable(test_time_stamp test_time_stamp.cc)
add_executable(test_log_sync test_log_sync.cc)
add_executable(test_log_async test_log_async.cc)
add_executable(test_timer_manager test_timer_manager.cc)
add_executable(test_event_loop test_event_loop.cc)
add_executable(test_eventloop_threadpool test_eventloop_threadpool.cc)
add_executable(test_socket1 test_socket1.cc)
add_executable(test_socket2 test_socket2.cc)
add_executable(test_buffer test_buffer.cc)

# link_libraries(mynetlib pthread)
target_link_libraries(test_log_sync mynetlib pthread)
target_link_libraries(test_log_async mynetlib pthread)
target_link_libraries(test_time_stamp mynetlib pthread)
target_link_libraries(test_timer_manager mynetlib pthread)
target_link_libraries(test_event_loop mynetlib pthread)
target_link_libraries(test_eventloop_threadpool mynetlib pthread)
target_link_libraries(test_socket1 mynetlib pthread)
target_link_libraries(test_socket2 mynetlib pthread)
target_link_libraries(test_buffer mynetlib pthread)
