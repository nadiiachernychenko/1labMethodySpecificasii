cmake_minimum_required(VERSION 3.14)
project(SysProgLexerTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(lexer_lib
    Lexer.cpp
)

target_include_directories(lexer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lexer_app
    main.cpp
)

target_link_libraries(lexer_app PRIVATE lexer_lib)

add_executable(lexer_tests
    LexerTests.cpp
)

target_link_libraries(lexer_tests
    PRIVATE
    lexer_lib
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(lexer_tests)