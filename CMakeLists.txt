cmake_minimum_required(VERSION 3.0)

project(gpiow)

find_package(pigpio REQUIRED)

add_library(gpiow
    src/error.c
    src/log.c
    src/multi_impl.c
    src/impl_pigpiod.c
)
target_compile_definitions(gpiow PUBLIC GPIOW_LOG_HOOK)
target_include_directories(gpiow PUBLIC include)
target_link_libraries(gpiow pigpio::pigpiod_if2)

add_executable(tsl2561 examples/tsl2561.c)
target_link_libraries(tsl2561 gpiow)
