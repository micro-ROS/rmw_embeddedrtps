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

#include "./types.hpp"

#include <rosidl_typesupport_microxrcedds_c/identifier.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./memory.hpp"

// Static memory pools

rmw_ertps_mempool_t session_memory;
rmw_context_impl_t custom_sessions[RMW_ERTPS_MAX_SESSIONS];

rmw_ertps_mempool_t node_memory;
rmw_ertps_node_t custom_nodes[RMW_ERTPS_MAX_NODES];

rmw_ertps_mempool_t publisher_memory;
rmw_ertps_publisher_t custom_publishers[RMW_ERTPS_MAX_PUBLISHERS];

rmw_ertps_mempool_t subscription_memory;
rmw_ertps_subscription_t custom_subscriptions[RMW_ERTPS_MAX_SUBSCRIPTIONS];

rmw_ertps_mempool_t service_memory;
rmw_ertps_service_t custom_services[RMW_ERTPS_MAX_SERVICES];

rmw_ertps_mempool_t client_memory;
rmw_ertps_client_t custom_clients[RMW_ERTPS_MAX_CLIENTS];

rmw_ertps_mempool_t static_buffer_memory;
rmw_ertps_static_input_buffer_t custom_static_buffers[RMW_ERTPS_MAX_HISTORY];

// Memory init functions

#define RMW_INIT_MEMORY(X) \
  void rmw_ertps_init_ ## X ## _memory( \
    rmw_ertps_mempool_t * memory, \
    rmw_ertps_ ## X ## _t * array, \
    size_t size) \
  { \
    sys_mutex_new(&memory->memory_mutex); \
    if (size > 0 && !memory->is_initialized) { \
      memory->is_initialized = true; \
      memory->element_size = sizeof(*array); \
      memory->allocateditems = NULL; \
      memory->freeitems = NULL; \
      memory->is_dynamic_allowed = true; \
 \
      for (size_t i = 0; i < size; i++) { \
        put_memory(memory, &array[i].mem); \
        array[i].mem.data = reinterpret_cast<void *>(&array[i]); \
        array[i].mem.is_dynamic_memory = false; \
      } \
    } \
  }


RMW_INIT_MEMORY(service)
RMW_INIT_MEMORY(client)
RMW_INIT_MEMORY(publisher)
RMW_INIT_MEMORY(subscription)
RMW_INIT_MEMORY(node)
RMW_INIT_MEMORY(session)
RMW_INIT_MEMORY(static_input_buffer)

rmw_ertps_mempool_item_t * rmw_ertps_find_static_input_buffer_by_owner(
  void * owner)
{
  rtps::Lock lock{static_buffer_memory.memory_mutex};

  rmw_ertps_mempool_item_t * static_buffer_item = static_buffer_memory.allocateditems;
  while (static_buffer_item != NULL) {
    rmw_ertps_static_input_buffer_t * data =
      reinterpret_cast<rmw_ertps_static_input_buffer_t *>(static_buffer_item->data);
    if (data->owner == owner) {
      return static_buffer_item;
    }
    static_buffer_item = static_buffer_item->next;
  }
  return NULL;
}
