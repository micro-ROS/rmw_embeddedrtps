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

#include <rmw/error_handling.h>
#include <rmw/allocators.h>

#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>

#include <micro_ros_utilities/type_utilities.h>
#include <micro_ros_utilities/string_utilities.h>

#include "./rmw_graph.hpp"
#include "./utils.hpp"

rmw_ret_t rmw_graph_publish(
  rmw_context_impl_t * context)
{
  const message_type_support_callbacks_t * functions = context->graph_type_support_callbacks;

  static uint8_t buffer[RMW_ERTPS_MAX_OUTPUT_BUFFER_SIZE];

  // Encapsulation
  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = 0;
  buffer[3] = 0;

  ucdrBuffer mb;
  ucdr_init_buffer(&mb, &buffer[4], sizeof(buffer) - 4);

  bool written = functions->cdr_serialize(&context->graph_info, &mb);

  if (written) {
    size_t size = ucdr_buffer_length(&mb) + 4;
    context->graph_writer->newChange(
      rtps::ChangeKind_t::ALIVE,
      buffer, size);
  } else {
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t rmw_graph_init(
  rmw_context_impl_t * context
)
{
  // Get typesupport
  const rosidl_message_type_support_t * type_support_xrce =
    get_message_typesupport_handle(
    ROSIDL_GET_MSG_TYPE_SUPPORT(rmw_dds_common, msg, ParticipantEntitiesInfo),
    ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);

  context->graph_type_support_callbacks =
    reinterpret_cast<const message_type_support_callbacks_t *>(type_support_xrce->data);

  // Init entities
  static char type_name[RMW_ERTPS_TYPE_NAME_MAX_LENGTH];
  generate_type_name(context->graph_type_support_callbacks, type_name, sizeof(type_name));

  context->graph_writer = context->domain->createWriter(
    *context->participant,
    "ros_discovery_info",
    type_name,
    true);

  if (nullptr == context->graph_writer) {
    return RMW_RET_ERROR;
  }

  context->graph_reader = context->domain->createReader(
    *context->participant,
    "ros_discovery_info",
    type_name,
    true);

  if (nullptr == context->graph_reader) {
    return RMW_RET_ERROR;
  }

  // Init message
  micro_ros_utilities_memory_rule_t rules[] = {
    {"node_entities_info_seq", 1},
    {"node_entities_info_seq.reader_gid_seq", rtps::Config::NUM_READERS_PER_PARTICIPANT},
    {"node_entities_info_seq.writer_gid_seq", rtps::Config::NUM_WRITERS_PER_PARTICIPANT},
  };

  micro_ros_utilities_memory_conf_t conf;
  conf.max_string_capacity = 256;
  conf.rules = rules;
  conf.n_rules = sizeof(rules) / sizeof(rules[0]);

  size_t required_size = micro_ros_utilities_get_static_size(
    type_support_xrce,
    conf
  );

  context->graph_info_buffer = reinterpret_cast<uint8_t *>( rmw_allocate(required_size));

  micro_ros_utilities_create_static_message_memory(
    type_support_xrce,
    &context->graph_info,
    conf,
    context->graph_info_buffer,
    required_size
  );

  // Fill participant info
  memcpy(context->graph_info.gid.data, context->participant->m_guidPrefix.id.data(), 12);
  context->graph_info.gid.data[12] = context->participant->m_participantId;
  context->graph_info.node_entities_info_seq.size = 0;

  rmw_graph_publish(context);

  return RMW_RET_OK;
}

rmw_ret_t rmw_graph_add_node(
  rmw_context_impl_t * context,
  const char * node_name,
  const char * node_namespace
)
{
  size_t i = context->graph_info.node_entities_info_seq.size;

  context->graph_info.node_entities_info_seq.data[i].node_name =
    micro_ros_string_utilities_set(
    context->graph_info.node_entities_info_seq.data[i].node_name,
    node_name);

  context->graph_info.node_entities_info_seq.data[i].node_namespace =
    micro_ros_string_utilities_set(
    context->graph_info.node_entities_info_seq.data[i].node_namespace,
    node_namespace);

  context->graph_info.node_entities_info_seq.size++;

  rmw_graph_publish(context);
}

rmw_ret_t rmw_graph_add_entity(
  rmw_context_impl_t * context,
  const char * node_name,
  rtps::Guid_t * gid,
  bool is_reader
)
{
  for (size_t i = 0; i < context->graph_info.node_entities_info_seq.size; i++) {
    if (0 == strcmp(node_name, context->graph_info.node_entities_info_seq.data[i].node_name.data)) {
      size_t j;
      uint8_t * data;

      if (is_reader) {
        j = context->graph_info.node_entities_info_seq.data[i].reader_gid_seq.size;
        context->graph_info.node_entities_info_seq.data[i].reader_gid_seq.size++;
        data = context->graph_info.node_entities_info_seq.data[i].reader_gid_seq.data[j].data;
      } else {
        j = context->graph_info.node_entities_info_seq.data[i].writer_gid_seq.size;
        context->graph_info.node_entities_info_seq.data[i].writer_gid_seq.size++;
        data = context->graph_info.node_entities_info_seq.data[i].writer_gid_seq.data[j].data;
      }

      std::copy(
        gid->prefix.id.begin(),
        gid->prefix.id.begin() + 12, data);
      std::copy(
        gid->prefix.id.begin() + 12,
        gid->prefix.id.begin() + 15, &data[12]);
      std::copy(
        gid->prefix.id.begin() + 15,
        gid->prefix.id.begin() + 16, &data[15]);

      break;
    }
  }

  rmw_graph_publish(context);
}
