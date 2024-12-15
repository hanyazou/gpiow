cmake_minimum_required(VERSION 3.0)

project(gpiow)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(PkgConfig REQUIRED)

# pigpio (https://github.com/smurfix/pigpio)
find_package(pigpio)
if(pigpio_FOUND)
    set(pigpio_src "src/impl_pigpiod.c")
endif()

# libmpsse (https://github.com/devttys0/libmpsse)
find_package(libmpsse REQUIRED)
if(libmpsse_FOUND)
    find_package(libftdi1 REQUIRED)
    pkg_check_modules(LIBFTDI1 REQUIRED libftdi1)
    set(libmpsse_src "src/impl_libmpsse.c")
endif()

add_library(gpiow SHARED
    src/error.c
    src/log.c
    src/uri.c
    src/multi_impl.c
    ${pigpio_src}
    ${libmpsse_src}
)
target_compile_definitions(gpiow PUBLIC GPIOW_LOG_HOOK)
target_include_directories(gpiow PUBLIC include)

if(pigpio_FOUND)
    target_link_libraries(gpiow PRIVATE pigpio::pigpiod_if2)
endif()
if(libmpsse_FOUND)
    target_include_directories(gpiow PRIVATE ${LIBFTDI1_INCLUDE_DIRS})
    target_link_directories(gpiow PRIVATE ${LIBFTDI1_LIBRARY_DIRS})
    target_link_libraries(gpiow PRIVATE ftdi1)
    target_include_directories(gpiow PRIVATE ${libmpsse_INCLUDE_DIRS})
    target_link_directories(gpiow PRIVATE ${libmpsse_LIBRARY_DIRS})
    target_link_libraries(gpiow PRIVATE mpsse)
endif()

#
# example executables
#
add_executable(tsl2561 examples/tsl2561.c)
target_link_libraries(tsl2561 gpiow)
