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

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <rmw/rmw.h>

#include "./types.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

size_t generate_type_name(
  const message_type_support_callbacks_t * members,
  char type_name[],
  size_t buffer_size);

int generate_topic_name(
  const char * topic_name,
  char * full_topic_name,
  size_t full_topic_name_size);

int generate_service_types(
  const service_type_support_callbacks_t * members,
  char * request_type,
  char * reply_type,
  size_t buffer_size);

int generate_service_topics(
  const char * service_name,
  char * request_topic,
  char * reply_topic,
  size_t buffer_size);

bool is_ertps_rmw_identifier_valid(
  const char * id);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_HPP_
