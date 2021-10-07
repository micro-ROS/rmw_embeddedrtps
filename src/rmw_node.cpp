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

#include <rmw_embeddedrtps/config.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./types.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

rmw_node_t *
rmw_create_node(
  rmw_context_t * context,
  const char * name,
  const char * namespace_)
{
  rmw_node_t * node_handle = NULL;
  if (!name || strlen(name) == 0) {
    RMW_SET_ERROR_MSG("name is null");
  } else if (!namespace_ || strlen(namespace_) == 0) {
    RMW_SET_ERROR_MSG("namespace is null");
  } else if (!context) {
    RMW_SET_ERROR_MSG("context is null");
  } else {
    rmw_ertps_mempool_item_t * memory_node = get_memory(&node_memory);

    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      return NULL;
    }

    rmw_ertps_node_t * node_info = reinterpret_cast<rmw_ertps_node_t *>(memory_node->data);
    node_info->context = context->impl;

    node_handle = rmw_node_allocate();
    if (!node_handle) {
      RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
      return NULL;
    }

    node_info->rmw_handle = node_handle;

    node_handle->implementation_identifier = rmw_get_implementation_identifier();
    node_handle->data = node_info;
    node_handle->name = reinterpret_cast<const char *>(
      rmw_allocate(sizeof(char) * (strlen(name) + 1)));

    if (!node_handle->name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      return NULL;
    }
    memcpy(const_cast<char *>(node_handle->name), name, strlen(name) + 1);

    node_handle->namespace_ = reinterpret_cast<const char *>(
      rmw_allocate(sizeof(char) * (strlen(namespace_) + 1)));
    if (!node_handle->namespace_) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      return NULL;
    }
    memcpy(const_cast<char *>(node_handle->namespace_), namespace_, strlen(namespace_) + 1);
  }

  return node_handle;
}

rmw_ret_t rmw_destroy_node(
  rmw_node_t * node)
{
  (void) node;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_node_assert_liveliness(
  const rmw_node_t * node)
{
  (void) node;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(
  const rmw_node_t * node)
{
  rmw_ertps_node_t * custom_node = reinterpret_cast<rmw_ertps_node_t *>(node->data);
  rmw_context_impl_t * context = custom_node->context;
  rmw_guard_condition_t * graph_guard_condition =
    &context->graph_guard_condition;

  return graph_guard_condition;
}

#ifdef __cplusplus
}
#endif
