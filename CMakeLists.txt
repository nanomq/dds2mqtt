cmake_minimum_required(VERSION 3.16.3)

project(dds2mqtt VERSION 1 LANGUAGES CXX C)

# Select a DDS as backend
option (DDS_BACKEND_CYCLONEDDS "Using cyclonedds as dds backend" OFF)
option (DDS_BACKEND_FASTDDS "Using fastdds as dds backend" ON)

# Find requirements for fastdds
if(DDS_BACKEND_FASTDDS)
  if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
  endif()
  if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
  endif()
endif(DDS_BACKEND_FASTDDS)

# Find requirements for cyclonedds
if(DDS_BACKEND_CYCLONEDDS)
  if(NOT TARGET CycloneDDS::ddsc)
    # Find the CycloneDDS package.
    find_package(CycloneDDS REQUIRED)
  endif()
endif(DDS_BACKEND_CYCLONEDDS)

# Find nanosdk requirements
find_package(nng CONFIG REQUIRED)

find_package(Threads)

file(GLOB DDS2MQTT_SOURCES "fastdds/*.cxx")
file(GLOB DDS2MQTT_SOURCES_CPP "fastdds/*.cpp")

#Check C++11
if(DDS_BACKEND_FASTDDS)
  include(CheckCXXCompilerFlag)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
      message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
  endif()

  message(STATUS "FASTDDS as backend for dds")

  file(GLOB DDS2MQTT_SOURCES "fastdds/*.cxx")
  file(GLOB DDS2MQTT_SOURCES_CPP "fastdds/*.cpp")
  message(STATUS DDS2MQTT_SOURCES)
endif(DDS_BACKEND_FASTDDS)

if(DDS_BACKEND_CYCLONEDDS)
  idlc_generate(TARGET MqttData_lib FILES "HelloWorldData.idl" WARNINGS no-implicit-extensibility)

  message(STATUS "FASTDDS as backend for dds")

  file(GLOB DDS2MQTT_SOURCES "cyclonedds/*.c")
  file(GLOB DDS2MQTT_SOURCES_CPP "cyclonedds/*.cpp")
endif(DDS_BACKEND_CYCLONEDDS)

add_executable(dds2mqtt ${DDS2MQTT_SOURCES} ${DDS2MQTT_SOURCES_CPP})
target_compile_definitions(dds2mqtt PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)

if(DDS_BACKEND_FASTDDS)
  target_link_libraries(dds2mqtt fastrtps fastcdr fastdds::optionparser nng)
endif(DDS_BACKEND_FASTDDS)

if(DDS_BACKEND_CYCLONEDDS)
  target_link_libraries(dds2mqtt MqttData_lib CycloneDDS::ddsc)
endif(DDS_BACKEND_CYCLONEDDS)
