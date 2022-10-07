cmake_minimum_required(VERSION 3.23)

project(file_transfer_hl C)

if(APPLE)
    add_definitions(-D_DARWIN_C_SOURCE)
endif()

set(CMAKE_C_STANDARD 17)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SOURCE_LIST ${SOURCE_DIR}/server.c
        ${SOURCE_DIR}/client.c
        ${SOURCE_DIR}/conversion.c)
set(HEADER_LIST ${INCLUDE_DIR}/conversion.h)

include_directories(${INCLUDE_DIR})

