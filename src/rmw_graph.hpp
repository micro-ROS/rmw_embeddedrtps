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

#ifndef RMW_GRAPH_HPP_
#define RMW_GRAPH_HPP_

#include <rtps/rtps.h>
#include <rtps/config.h>

#include <rmw/types.h>

#include <rmw_dds_common/msg/participant_entities_info.h>

#ifdef __cplusplus
extern "C"
{
#endif

rmw_ret_t rmw_graph_init(
  rmw_context_impl_t * context);

rmw_ret_t rmw_graph_add_entity(
  rmw_context_impl_t * context,
  const char * node_name,
  const rtps::Guid_t * gid,
  const bool is_reader);

#ifdef __cplusplus
}
#endif

#endif  // RMW_GRAPH_HPP_