cmake_minimum_required(VERSION 3.16.3)

project(dds2mqtt VERSION 1 LANGUAGES CXX)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

find_package(nng REQUIRED)

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring dds2mqtt...")
file(GLOB DDS2MQTT_SOURCES_CXX "src/*.cxx")
file(GLOB DDS2MQTT_SOURCES_CPP "src/*.cpp")

add_executable(dds2mqtt ${DDS2MQTT_SOURCES_CXX} ${DDS2MQTT_SOURCES_CPP})
target_compile_definitions(dds2mqtt PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)

target_link_libraries(dds2mqtt fastrtps fastcdr fastdds::optionparser)
