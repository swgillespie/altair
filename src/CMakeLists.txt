add_executable(gen_magics gen_magics.cc)
target_compile_features(gen_magics PUBLIC cxx_std_20)

add_library(altair_lib
  uci.cc uci.h
  position.cc position.h
  movegen.cc movegen.h
  attacks.cc attacks.h
  bitboard.cc bitboard.h
  magics.cc
  thread.cc thread.h
  search.cc search.h
  eval.cc eval.h
  value.cc value.h
)
target_compile_features(altair_lib PUBLIC cxx_std_20)
set_property(TARGET altair_lib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET altair_lib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)

add_executable(altair main.cc)
target_compile_features(altair PUBLIC cxx_std_20)
target_link_libraries(altair altair_lib)
set_property(TARGET altair PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET altair PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)

add_executable(altair_test bitboard_test.cc position_test.cc movegen_test.cc)
target_include_directories(altair_test SYSTEM PRIVATE ${googletest_SOURCE_DIR}/googletest/include PRIVATE ${googletest_SOURCE_DIR}/googletest)
target_compile_features(altair_test PUBLIC cxx_std_20)
target_link_libraries(altair_test altair_lib gtest gtest_main)
set_property(TARGET altair_test PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET altair_test PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
gtest_discover_tests(altair_test)
