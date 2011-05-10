cmake_minimum_required(VERSION 2.6)
project(raku)

# Set the C compiler's flags for GCC: optimize and generate debug info.
set(CMAKE_C_FLAGS "-g -O")

#set(LIBS ${LIBS} pthread m)

include_directories(include)
add_subdirectory(src build)
add_subdirectory(test test) 
file(GLOB TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/test_*.c)
message(STATUS ${TESTS})

# For testing
# set(TEST_LIBS ${LIBS} rtcutl m)
# set(TEST_NAME rtcutl_test)
# add_executable(${TEST_NAME} ${TEST_NAME}.c)
# target_link_libraries(${TEST_NAME} ${TEST_LIBS})
# set(TEST_EXE ${EXECUTABLE_OUTPUT_PATH}${TEST_NAME}) 
# # Make check compiles the tests and runs them immediately.
# add_custom_target(check COMMAND ${TEST_EXE} DEPENDS ${TEST_EXE})
