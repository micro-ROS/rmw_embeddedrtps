# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0057 NEW)

set(PACKAGE_VERSION "0.0.1")
project(rmw_embeddedrtps
  VERSION
    ${PACKAGE_VERSION}
  LANGUAGES
    C CXX
)

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)
find_package(rosidl_runtime_c REQUIRED)
find_package(embeddedrtps REQUIRED)
find_package(microcdr REQUIRED)
find_package(rmw REQUIRED)

# Build options
option(RMW_ERTPS_GRAPH "Allows to perform graph-related operations to the user" OFF)

set(RMW_ERTPS_MAX_HISTORY "10" CACHE STRING 
"This value sets the number of history slots available for RMW subscriptions, requests and replies")
set(RMW_ERTPS_MAX_INPUT_BUFFER_SIZE "1000" CACHE STRING "TODO")
set(RMW_ERTPS_MAX_OUTPUT_BUFFER_SIZE "1000" CACHE STRING "TODO")

set(RMW_ERTPS_MAX_SESSIONS "1" CACHE STRING 
set(RMW_ERTPS_MAX_NODES "1" CACHE STRING "This value sets the maximum number of nodes.")
set(RMW_ERTPS_MAX_PUBLISHERS "1" CACHE STRING "This value sets the maximum number of publishers for an application.")
set(RMW_ERTPS_MAX_SUBSCRIPTIONS "1" CACHE STRING "This value sets the maximum number of subscriptions for an application.")
set(RMW_ERTPS_MAX_SERVICES "1" CACHE STRING "This value sets the maximum number of services for an application.")
set(RMW_ERTPS_MAX_CLIENTS "1" CACHE STRING "This value sets the maximum number of clients for an application.")
set(RMW_ERTPS_TOPIC_NAME_MAX_LENGTH "200" CACHE STRING "This value sets the maximum number of characters for a topic name.")
set(RMW_ERTPS_TYPE_NAME_MAX_LENGTH "200" CACHE STRING "This value sets the maximum number of characters for a type name.")

# Create source files with the define
configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in
  ${PROJECT_BINARY_DIR}/include/rmw_embeddedrtps/config.h)

set(SRCS
  src/identifiers.c
  src/memory.cpp
  src/rmw_client.cpp
  src/rmw_compare_gids_equal.c
  src/rmw_count.cpp
  src/rmw_event.c
  src/rmw_get_gid_for_publisher.cpp
  src/rmw_get_implementation_identifier.c
  src/rmw_get_serialization_format.c
  src/rmw_get_topic_endpoint_info.cpp
  src/rmw_get_endpoint_network_flow.c
  src/rmw_qos_profile_check_compatible.c
  src/rmw_guard_condition.c
  src/rmw_init.cpp
  src/rmw_logging.c
  src/rmw_node.cpp
  src/rmw_node_info_and_types.cpp
  src/rmw_node_names.cpp
  src/rmw_publish.cpp
  src/rmw_publisher.cpp
  src/rmw_request.cpp
  src/rmw_response.cpp
  src/rmw_serialize.c
  src/rmw_service.cpp
  src/rmw_service_names_and_types.cpp
  src/rmw_service_server_is_available.c
  src/rmw_subscription.cpp
  src/rmw_take.cpp
  src/rmw_topic_names_and_types.cpp
  src/rmw_trigger_guard_condition.c
  src/rmw_wait.cpp
  src/rmw_wait_set.c
  src/types.cpp
  src/utils.cpp
  src/callbacks.cpp
  $<$<BOOL:${RMW_ERTPS_GRAPH}>:src/rmw_graph.cpp>
)

add_library(${PROJECT_NAME}
  ${SRCS})

if(RMW_ERTPS_GRAPH)
  find_package(rmw_dds_common REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME}
  microcdr
  embeddedrtps
  $<$<BOOL:${RMW_ERTPS_GRAPH}>:rmw_dds_common>
)

# Type support
find_package(rosidl_typesupport_microxrcedds_c)
ament_target_dependencies(${PROJECT_NAME}
  "rosidl_typesupport_microxrcedds_c")

ament_target_dependencies(${PROJECT_NAME}
  "rcutils"
  "rmw"
  "embeddedrtps"
)

configure_rmw_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD
    99
  C_STANDARD_REQUIRED
    YES
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wextra>
    $<$<C_COMPILER_ID:GNU>:-pedantic>
    $<$<C_COMPILER_ID:GNU>:-Werror-implicit-function-declaration>
    $<$<C_COMPILER_ID:GNU>:$<$<VERSION_GREATER:$<C_COMPILER_VERSION>,8.1>:-Wcast-align=strict>>
    $<$<C_COMPILER_ID:GNU>:-Wvla>
    $<$<C_COMPILER_ID:MSVC>:/Wall>
)

file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)

ament_export_include_directories(${CMAKE_INSTALL_PREFIX}/include)
ament_export_include_directories(${embeddedrtps_INCLUDE_DIR})
ament_export_include_directories(${microcdr_INCLUDE_DIR})
ament_export_libraries(${PROJECT_NAME})

ament_export_dependencies(
  rcutils
  rmw
  embeddedrtps
)

ament_export_dependencies(rosidl_typesupport_microxrcedds_c)
set(implementations "c:rosidl_typesupport_c:rosidl_typesupport_microxrcedds_c")

register_rmw_implementation(${implementations})

if(BUILD_TESTING)
  # Pedantic in CI
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

# Install config.h file.
install(
  FILES
    ${PROJECT_BINARY_DIR}/include/rmw_embeddedrtps/config.h
  DESTINATION
    include/rmw_embeddedrtps
)

# Install library.
install(
  TARGETS
    ${PROJECT_NAME}
  ARCHIVE DESTINATION
    lib
  LIBRARY DESTINATION
    lib
  RUNTIME DESTINATION
    bin
)