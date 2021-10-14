// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <rtps/rtps.h>
#include <rtps/common/types.h>

#include <stddef.h>

#include <rmw/types.h>
#include <ucdr/microcdr.h>

#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>

#include <rosidl_runtime_c/service_type_support_struct.h>
#include <rosidl_typesupport_microxrcedds_c/service_type_support.h>

#include <rmw_embeddedrtps/config.h>
#include <rmw/error_handling.h>

#include "./memory.hpp"

extern "C" {

typedef struct rmw_context_impl_t
{
  rmw_ertps_mempool_item_t mem;

  rtps::Domain * domain;
  rtps::Participant * participant;

  rmw_guard_condition_t graph_guard_condition;

#ifdef RMW_ERTPS_GRAPH
  rtps::Writer * graph_writer;
  rtps::Reader * graph_reader;

  const message_type_support_callbacks_t * graph_type_support_callbacks;

  rmw_dds_common__msg__ParticipantEntitiesInfo graph_info;
  uint8_t * graph_info_buffer;
#endif  // RMW_ERTPS_GRAPH
} rmw_context_impl_t;

typedef struct rmw_context_impl_t rmw_ertps_session_t;

// ROS2 entities definitions

typedef struct rmw_ertps_service_t
{
  rmw_ertps_mempool_item_t mem;
  rmw_service_t * rmw_handle;

  const service_type_support_callbacks_t * type_support_callbacks;

  rmw_qos_profile_t qos;

  rtps::Writer * writer;
  rtps::Reader * reader;

  struct rmw_ertps_node_t * owner_node;

  bool has_data;
} rmw_ertps_service_t;

typedef struct rmw_ertps_client_t
{
  rmw_ertps_mempool_item_t mem;
  rmw_client_t * rmw_handle;

  const service_type_support_callbacks_t * type_support_callbacks;

  rmw_qos_profile_t qos;

  rtps::Writer * writer;
  rtps::Reader * reader;

  struct rmw_ertps_node_t * owner_node;

  bool has_data;
} rmw_ertps_client_t;

typedef struct rmw_ertps_subscription_t
{
  rmw_ertps_mempool_item_t mem;
  rmw_subscription_t * rmw_handle;

  const message_type_support_callbacks_t * type_support_callbacks;

  rmw_qos_profile_t qos;

  rtps::Reader * reader;

  struct rmw_ertps_node_t * owner_node;
  bool has_data;
} rmw_ertps_subscription_t;

typedef struct rmw_ertps_publisher_t
{
  rmw_ertps_mempool_item_t mem;
  rmw_publisher_t * rmw_handle;

  const message_type_support_callbacks_t * type_support_callbacks;

  rmw_qos_profile_t qos;

  rtps::Writer * writer;

  struct rmw_ertps_node_t * owner_node;
} rmw_ertps_publisher_t;

typedef struct rmw_ertps_node_t
{
  rmw_ertps_mempool_item_t mem;
  rmw_node_t * rmw_handle;
  rmw_context_impl_t * context;
} rmw_ertps_node_t;

typedef struct rmw_ertps_static_input_buffer_t
{
  rmw_ertps_mempool_item_t mem;

  uint8_t buffer[RMW_ERTPS_MAX_INPUT_BUFFER_SIZE];
  size_t length;
  void * owner;

  rtps::Guid_t writer_guid;
  rtps::SequenceNumber_t sequence_number;

  rtps::Guid_t related_writer_guid;
  rtps::SequenceNumber_t related_sequence_number;
} rmw_ertps_static_input_buffer_t;

// Static memory pools

extern rmw_ertps_mempool_t session_memory;
extern rmw_context_impl_t custom_sessions[RMW_ERTPS_MAX_DOMAINS];

extern rmw_ertps_mempool_t node_memory;
extern rmw_ertps_node_t custom_nodes[ERTPS_MAX_PARTICIPANTS];

extern rmw_ertps_mempool_t publisher_memory;
extern rmw_ertps_publisher_t custom_publishers[ERTPS_MAX_PUBLISHERS];

extern rmw_ertps_mempool_t subscription_memory;
extern rmw_ertps_subscription_t custom_subscriptions[ERTPS_MAX_SUBSCRIPTIONS];

extern rmw_ertps_mempool_t service_memory;
extern rmw_ertps_service_t custom_services[ERTPS_MAX_SERVICES];

extern rmw_ertps_mempool_t client_memory;
extern rmw_ertps_client_t custom_clients[ERTPS_MAX_CLIENTS];

extern rmw_ertps_mempool_t static_buffer_memory;
extern rmw_ertps_static_input_buffer_t custom_static_buffers[ERTPS_MAX_HISTORY];

// Memory init functions

#define RMW_INIT_DEFINE_MEMORY(X) \
  void rmw_ertps_init_ ## X ## _memory( \
    rmw_ertps_mempool_t * memory, \
    rmw_ertps_ ## X ## _t * array, \
    size_t size);

RMW_INIT_DEFINE_MEMORY(service)
RMW_INIT_DEFINE_MEMORY(client)
RMW_INIT_DEFINE_MEMORY(publisher)
RMW_INIT_DEFINE_MEMORY(subscription)
RMW_INIT_DEFINE_MEMORY(node)
RMW_INIT_DEFINE_MEMORY(session)
RMW_INIT_DEFINE_MEMORY(static_input_buffer)

// Memory management functions
rmw_ertps_mempool_item_t * rmw_ertps_find_static_input_buffer_by_owner(
  void * owner);

}

#endif  // TYPES_HPP_
