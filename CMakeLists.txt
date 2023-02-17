cmake_minimum_required(VERSION 3.7...3.25)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(altair VERSION 0.1.0
            DESCRIPTION "Altair chess engine"
            LANGUAGES CXX)
include(CheckIPOSupported)
include(FetchContent)
include(GoogleTest)
set(GOOGLETEST_VERSION "1.13.0")

check_ipo_supported()
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        "v${GOOGLETEST_VERSION}"
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
endif(NOT googletest_POPULATED)

set(is_clang "$<CXX_COMPILER_ID:Clang>")
set(is_clang_like "$<CXX_COMPILER_ID:Clang,GNU>")
set(is_release "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")

add_compile_options(
  $<$<AND:$<CONFIG:Debug>,${is_clang}>:-fsanitize=address>
  $<$<AND:$<CONFIG:RelWithDebInfo>,${is_clang_like}>:-fno-omit-frame-pointer>
  $<${is_clang}:-fdiagnostics-color=always>
  $<${is_clang_like}:-Wall>
)

add_link_options(
  $<$<CONFIG:Debug>:-fsanitize=address>
)

add_subdirectory(${googletest_SOURCE_DIR}/googletest ${googletest_BINARY_DIR})
add_subdirectory(src)
