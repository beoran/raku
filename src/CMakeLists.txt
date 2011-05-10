
file(GLOB RAKU_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
add_library(raku SHARED ${RAKU_SRC})
target_link_libraries(raku ${LIBS})


