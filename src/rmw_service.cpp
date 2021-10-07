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

#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#include <rosidl_typesupport_microxrcedds_c/service_type_support.h>

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./utils.hpp"
#include "./callbacks.hpp"

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_policies)
{
  rmw_service_t * rmw_service = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (!is_ertps_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!service_name || strlen(service_name) == 0) {
    RMW_SET_ERROR_MSG("service name is null or empty string");
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
  } else {
    rmw_service = reinterpret_cast<rmw_service_t *>(rmw_allocate(
        sizeof(rmw_service_t)));
    rmw_service->data = NULL;
    rmw_service->implementation_identifier = rmw_get_implementation_identifier();

    rmw_service->service_name =
      (const char *)(rmw_allocate(sizeof(char) * (strlen(service_name) + 1)));
    if (!rmw_service->service_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    memcpy(
      const_cast<char *>(rmw_service->service_name),
      service_name, strlen(service_name) + 1);

    rmw_ertps_node_t * custom_node = reinterpret_cast<rmw_ertps_node_t *>(node->data);
    rmw_ertps_mempool_item_t * memory_node = get_memory(&service_memory);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    rmw_ertps_service_t * custom_service =
      reinterpret_cast<rmw_ertps_service_t *>(memory_node->data);
    custom_service->rmw_handle = rmw_service;

    custom_service->owner_node = custom_node;
    custom_service->qos = *qos_policies;

    const rosidl_service_type_support_t * type_support_xrce = get_service_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);

    if (NULL == type_support_xrce) {
      RMW_SET_ERROR_MSG("Undefined type support");
      goto fail;
    }

    custom_service->type_support_callbacks =
      reinterpret_cast<const service_type_support_callbacks_t *>(type_support_xrce->data);

    if (custom_service->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    }

    char req_type_name[RMW_ERTPS_TYPE_NAME_MAX_LENGTH];
    char res_type_name[RMW_ERTPS_TYPE_NAME_MAX_LENGTH];
    generate_service_types(
      custom_service->type_support_callbacks, req_type_name, res_type_name,
      RMW_ERTPS_TYPE_NAME_MAX_LENGTH);

    char req_topic_name[RMW_ERTPS_TOPIC_NAME_MAX_LENGTH];
    char res_topic_name[RMW_ERTPS_TOPIC_NAME_MAX_LENGTH];
    generate_service_topics(
      service_name, req_topic_name, res_topic_name,
      RMW_ERTPS_TOPIC_NAME_MAX_LENGTH);

    custom_service->writer = custom_node->context->domain->createWriter(
      *custom_node->context->participant,
      res_topic_name,
      res_type_name,
      qos_policies->reliability != RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);

    if (nullptr == custom_service->writer) {
      goto fail;
    }

    custom_service->reader = custom_node->context->domain->createReader(
      *custom_node->context->participant,
      req_topic_name,
      req_type_name,
      qos_policies->reliability != RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);

    if (nullptr == custom_service->reader) {
      goto fail;
    }

    custom_service->reader->registerCallback(
      generic_callback<rmw_ertps_service_t>,
      custom_service->reader);


    rmw_service->data = custom_service;
  }
  return rmw_service;

fail:
  rmw_service = NULL;
  return rmw_service;
}

rmw_ret_t
rmw_destroy_service(
  rmw_node_t * node,
  rmw_service_t * service)
{
  (void) node;
  (void) service;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}
