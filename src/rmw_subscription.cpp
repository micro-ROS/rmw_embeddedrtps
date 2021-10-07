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

#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>

#include <rmw/rmw.h>
#include <rmw/error_handling.h>
#include <rmw/types.h>
#include <rmw/allocators.h>

#include "./utils.hpp"
#include "./callbacks.hpp"

rmw_ret_t
rmw_init_subscription_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_runtime_c__Sequence__bound * message_bounds,
  rmw_subscription_allocation_t * allocation)
{
  (void)type_support;
  (void)message_bounds;
  (void)allocation;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_fini_subscription_allocation(
  rmw_subscription_allocation_t * allocation)
{
  (void)allocation;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
  const rmw_subscription_options_t * subscription_options)
{
  (void)subscription_options;

  rmw_subscription_t * rmw_subscription = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (!is_ertps_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("subscription topic is null or empty string");
    return NULL;
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return NULL;
  } else {
    rmw_subscription = reinterpret_cast<rmw_subscription_t *>(rmw_allocate(
        sizeof(rmw_subscription_t)));
    rmw_subscription->data = NULL;
    rmw_subscription->implementation_identifier = rmw_get_implementation_identifier();
    size_t topic_name_size = strlen(topic_name) + 1;
    rmw_subscription->topic_name =
      (const char *)(rmw_allocate(sizeof(char) * topic_name_size));
    if (!rmw_subscription->topic_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    snprintf(const_cast<char *>(rmw_subscription->topic_name), topic_name_size, "%s", topic_name);

    rmw_ertps_node_t * custom_node = reinterpret_cast<rmw_ertps_node_t *>(node->data);
    rmw_ertps_mempool_item_t * memory_node = get_memory(&subscription_memory);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    rmw_ertps_subscription_t * custom_subscription =
      reinterpret_cast<rmw_ertps_subscription_t *>(memory_node->data);
    custom_subscription->rmw_handle = rmw_subscription;

    custom_subscription->owner_node = custom_node;
    memcpy(&custom_subscription->qos, qos_policies, sizeof(rmw_qos_profile_t));

    const rosidl_message_type_support_t * type_support_xrce = get_message_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);

    if (NULL == type_support_xrce) {
      RMW_SET_ERROR_MSG("Undefined type support");
      goto fail;
    }

    custom_subscription->type_support_callbacks =
      reinterpret_cast<const message_type_support_callbacks_t *>(type_support_xrce->data);

    if (custom_subscription->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    }

    static char full_topic_name[RMW_ERTPS_TOPIC_NAME_MAX_LENGTH];
    static char type_name[RMW_ERTPS_TYPE_NAME_MAX_LENGTH];

    generate_topic_name(topic_name, full_topic_name, sizeof(full_topic_name));
    generate_type_name(custom_subscription->type_support_callbacks, type_name, sizeof(type_name));

    custom_subscription->reader = custom_node->context->domain->createReader(
      *custom_node->context->participant,
      full_topic_name,
      type_name,
      qos_policies->reliability != RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);

    if (nullptr == custom_subscription->reader) {
      goto fail;
    }

#ifdef RMW_ERTPS_GRAPH
    rmw_graph_add_entity(
      custom_node->context,
      custom_node->rmw_handle->name,
      &custom_subscription->reader->m_attributes.endpointGuid,
      true
    );
#endif  // RMW_ERTPS_GRAPH

    custom_subscription->reader->registerCallback(
      generic_callback<rmw_ertps_subscription_t>,
      custom_subscription->reader);

    rmw_subscription->data = custom_subscription;
  }
  return rmw_subscription;

fail:
  rmw_subscription = NULL;
  return rmw_subscription;
}

rmw_ret_t
rmw_subscription_count_matched_publishers(
  const rmw_subscription_t * subscription,
  size_t * publisher_count)
{
  (void)subscription;
  (void)publisher_count;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_subscription_get_actual_qos(
  const rmw_subscription_t * subscription,
  rmw_qos_profile_t * qos)
{
  (void)qos;

  rmw_ertps_subscription_t * custom_subscription =
    reinterpret_cast<rmw_ertps_subscription_t *>(subscription->data);
  qos = &custom_subscription->qos;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_destroy_subscription(
  rmw_node_t * node,
  rmw_subscription_t * subscription)
{
  (void) node;
  (void) subscription;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}
