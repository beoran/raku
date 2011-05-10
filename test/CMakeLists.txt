
file(GLOB RAKU_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test_*.c)

foreach(TEST_CFILE ${RAKU_TESTS}) 
  get_filename_component(TEST_EXE TEST_CFILE NAME_WE)
  add_executable(${TEST_EXE} ${TEST_CFILE})
  add_test(${TEST_EXE} ${TEST_EXE})
endforeach(TEST_CFILE)



