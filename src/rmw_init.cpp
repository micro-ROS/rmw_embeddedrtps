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

#include <time.h>

#include <rtps/rtps.h>

#include <rmw_embeddedrtps/config.h>
#include <rmw/rmw.h>
#include <rmw/error_handling.h>
#include <rmw/allocators.h>

#include "./types.hpp"
#include "./utils.hpp"

#include "./identifiers.h"

rmw_ret_t
rmw_init_options_init(
  rmw_init_options_t * init_options,
  rcutils_allocator_t allocator)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);

  if (NULL != init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized init_options");

    return RMW_RET_INVALID_ARGUMENT;
  }

  init_options->instance_id = 0;
  init_options->implementation_identifier = embeddedrtps_identifier;
  init_options->allocator = allocator;
  init_options->enclave = "/";
  init_options->domain_id = 0;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_options_copy(
  const rmw_init_options_t * src,
  rmw_init_options_t * dst)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(src, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(dst, RMW_RET_INVALID_ARGUMENT);

  if (NULL != dst->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized dst");

    return RMW_RET_INVALID_ARGUMENT;
  }
  memcpy(dst, src, sizeof(rmw_init_options_t));

  return RMW_RET_OK;
}

void matchedPub(void * args)
{
  (void) args;
  // printf("Matched Pub: %s:%d\n", __FILE__, __LINE__);
}

void matchedSub(void * args)
{
  (void) args;
  // printf("Matched Sub: %s:%d\n", __FILE__, __LINE__);
}

rmw_ret_t
rmw_init_options_fini(
  rmw_init_options_t * init_options)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&(init_options->allocator), return RMW_RET_INVALID_ARGUMENT);

  *init_options = rmw_get_zero_initialized_init_options();
  return RMW_RET_OK;
}

rmw_ret_t
rmw_init(
  const rmw_init_options_t * options,
  rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  // RCUTILS_CHECK_ARGUMENT_FOR_NULL(options->impl, RMW_RET_INVALID_ARGUMENT);

  context->instance_id = options->instance_id;
  context->implementation_identifier = embeddedrtps_identifier;
  context->actual_domain_id = options->domain_id;

  rmw_ertps_init_session_memory(&session_memory, custom_sessions, RMW_ERTPS_MAX_DOMAINS);
  rmw_ertps_init_static_input_buffer_memory(
    &static_buffer_memory, custom_static_buffers,
    RMW_ERTPS_MAX_HISTORY);

  rmw_ertps_mempool_item_t * memory_node = get_memory(&session_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available session memory node");

    return RMW_RET_ERROR;
  }

  rmw_context_impl_t * context_impl = reinterpret_cast<rmw_context_impl_t *>(memory_node->data);

  extern sys_sem_t rmw_wait_sem;
  sys_sem_new(&rmw_wait_sem, 0);

  // TODO: add domain number check?
  context_impl->domain = new rtps::Domain(options->domain_id);
  context_impl->participant = context_impl->domain->createParticipant();
  context->impl = context_impl;

#ifdef RMW_ERTPS_GRAPH
  rmw_graph_init(context_impl);
#endif  // RMW_ERTPS_GRAPH

  if (!context_impl->domain->completeInit()) {
    return RMW_RET_ERROR;
  }

  rmw_ertps_init_node_memory(&node_memory, custom_nodes, RMW_ERTPS_MAX_NODES);
  rmw_ertps_init_subscription_memory(
    &subscription_memory, custom_subscriptions,
    RMW_ERTPS_MAX_SUBSCRIPTIONS);
  rmw_ertps_init_publisher_memory(&publisher_memory, custom_publishers, RMW_ERTPS_MAX_PUBLISHERS);
  rmw_ertps_init_service_memory(&service_memory, custom_services, RMW_ERTPS_MAX_SERVICES);
  rmw_ertps_init_client_memory(&client_memory, custom_clients, RMW_ERTPS_MAX_CLIENTS);

  if (nullptr == context_impl->participant) {
    return RMW_RET_ERROR;
  }

  // Register callback to ensure that a publisher is matched to the writer before sending messages
  context_impl->participant->registerOnNewPublisherMatchedCallback(matchedPub, nullptr);
  context_impl->participant->registerOnNewSubscriberMatchedCallback(matchedSub, nullptr);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_shutdown(
  rmw_context_t * context)
{
  (void) context;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_context_fini(
  rmw_context_t * context)
{
  (void) context;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}
