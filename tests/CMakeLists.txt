cmake_minimum_required(VERSION 3.14)
project(picoboot_testing)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()


# flash_util_test
add_executable(flash_util_test FlashUtilTest.cpp ../bootutil/src/flash_util.c)
target_include_directories(flash_util_test PRIVATE ../bootutil/inc stubs)
target_link_libraries(flash_util_test GTest::gtest_main)

# crc32_test
add_executable(crc32_test Crc32Test.cpp ../bootutil/src/crc32.c)
target_include_directories(crc32_test PRIVATE ../bootutil/inc)
target_link_libraries(crc32_test GTest::gtest_main)




include(GoogleTest)
gtest_discover_tests(
  flash_util_test
  crc32_test
)