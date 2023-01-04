cmake_minimum_required(VERSION 3.16.3)

project(dds2mqtt VERSION 1 LANGUAGES C)

# Select a DDS as backend
option (DEBUG "ENABLE DEBUG MODE" ON)
option (TEST  "ENABLE UNIT TEST" OFF)

if(DEBUG)
  SET(CMAKE_C_FLAGS "-g -Wall")
endif(DEBUG)

# Find requirements for cyclonedds
if(NOT TARGET CycloneDDS::ddsc)
  # Find the CycloneDDS package.
  find_package(CycloneDDS REQUIRED)
endif()

# Find nanosdk requirements
find_package(nng CONFIG REQUIRED)

find_package(Threads)

idlc_generate(TARGET HelloWorld_lib FILES "src/HelloWorld.idl" WARNINGS no-implicit-extensibility)
message(STATUS "CycloneDDS as backend for dds")

file(GLOB DDS2MQTT_SOURCES "src/*.c")

add_executable(dds2mqtt ${DDS2MQTT_SOURCES})
target_compile_definitions(dds2mqtt PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)

target_link_libraries(dds2mqtt HelloWorld_lib CycloneDDS::ddsc nng)

if(TEST)
  set(UNIT_TEST_PATH ${CMAKE_SOURCE_DIR}/test)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/tests)

  add_executable(dds2mqtt_test test/test.c src/HelloWorldMQTTTypes.c)
  target_include_directories(dds2mqtt_test PRIVATE src)
  target_link_libraries(dds2mqtt_test HelloWorld_lib ${CMAKE_THREAD_LIBS_INIT} CycloneDDS::ddsc nng)
  add_test(dds2mqtt_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dds2mqtt_test)
endif(TEST)

