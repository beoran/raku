
set (RAKU_SRC
  brobject.c
)
add_library(libraku SHARED ${RAKU_SRC})
target_link_libraries(libraku ${LIBS})
add_executable(raku ${RAKU_SRC} main.c)
target_link_libraries(raku libraku ${LIBS})


