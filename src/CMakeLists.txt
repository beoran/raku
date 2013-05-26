
set (RAKU_SRC
  bstrlib.c
  bstraux.c
  slre.c
  raparser.c
  raku.c
  main.c
)
add_library(rakulib SHARED ${RAKU_SRC})
target_link_libraries(rakulib ${LIBS})
add_executable(raku ${RAKU_SRC} main.c)
target_link_libraries(raku rakulib ${LIBS})


