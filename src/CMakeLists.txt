add_executable(gen_magics gen_magics.cc)
target_compile_features(gen_magics PUBLIC cxx_std_20)

add_library(altair_lib uci.cc position.cc movegen.cc attacks.cc bitboard.cc magics.cc thread.cc search.cc)
target_compile_features(altair_lib PUBLIC cxx_std_20)

add_executable(altair main.cc)
target_compile_features(altair PUBLIC cxx_std_20)
target_link_libraries(altair altair_lib)

add_executable(altair_test bitboard_test.cc position_test.cc movegen_test.cc)
target_include_directories(altair_test SYSTEM PRIVATE ${googletest_SOURCE_DIR}/googletest/include PRIVATE ${googletest_SOURCE_DIR}/googletest)
target_compile_features(altair_test PUBLIC cxx_std_20)
target_link_libraries(altair_test altair_lib gtest gtest_main)
gtest_discover_tests(altair_test)