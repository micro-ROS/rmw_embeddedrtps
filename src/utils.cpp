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

#include "./utils.hpp"

#include <rmw/error_handling.h>

#include "./types.hpp"

static const char ros_topic_prefix[] = "rt";
static const char ros_request_prefix[] = "rq";
static const char ros_reply_prefix[] = "rr";
static const char ros_request_subfix[] = "Request";
static const char ros_reply_subfix[] = "Reply";

int generate_service_topics(
  const char * service_name,
  char * request_topic,
  char * reply_topic,
  size_t buffer_size)
{
  snprintf(
    request_topic, buffer_size, "%s%s%s", ros_request_prefix,
    service_name, ros_request_subfix);

  snprintf(
    reply_topic, buffer_size, "%s%s%s", ros_reply_prefix,
    service_name, ros_reply_subfix);

  return 1;
}

int generate_service_types(
  const service_type_support_callbacks_t * members,
  char * request_type,
  char * reply_type,
  size_t buffer_size)
{
  const rosidl_message_type_support_t * req_members = members->request_members_();
  const rosidl_message_type_support_t * res_members = members->response_members_();

  const message_type_support_callbacks_t * req_callbacks =
    reinterpret_cast<const message_type_support_callbacks_t *>(req_members->data);
  const message_type_support_callbacks_t * res_callbacks =
    reinterpret_cast<const message_type_support_callbacks_t *>(res_members->data);

  generate_type_name(req_callbacks, request_type, buffer_size);
  generate_type_name(res_callbacks, reply_type, buffer_size);

  return 0;
}

size_t generate_type_name(
  const message_type_support_callbacks_t * members,
  char type_name[],
  size_t buffer_size)
{
  static const char * sep = "::";
  static const char * protocol = "dds";
  static const char * suffix = "_";
  size_t full_name_size =
    strlen(protocol) +
    strlen(suffix) +
    strlen(sep) +
    strlen(members->message_name_) +
    strlen(suffix) +
    ((NULL != members->message_namespace_) ? strlen(members->message_namespace_) : 0) + 1;

  type_name[0] = 0;

  snprintf(
    type_name, buffer_size,
    "%s%s%s%s%s%s%s",
    (NULL != members->message_namespace_) ? members->message_namespace_ : "",
    (NULL != members->message_namespace_) ? sep : "",
    protocol,
    suffix,
    sep,
    members->message_name_,
    suffix
  );

  return full_name_size;
}

int generate_topic_name(
  const char * topic_name,
  char * full_topic_name,
  size_t full_topic_name_size)
{
  int ret = snprintf(
    full_topic_name,
    full_topic_name_size,
    "%s%s",
    ros_topic_prefix,
    topic_name);
  if ((ret < 0) && (ret >= static_cast<int>(full_topic_name_size))) {
    return 0;
  }
  return ret;
}

bool is_ertps_rmw_identifier_valid(
  const char * id)
{
  return id != NULL &&
         strcmp(id, rmw_get_implementation_identifier()) == 0;
}
