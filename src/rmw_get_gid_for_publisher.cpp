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

#include <rmw/rmw.h>
#include <rmw/error_handling.h>

#include "./types.hpp"

rmw_ret_t
rmw_get_gid_for_publisher(
  const rmw_publisher_t * publisher,
  rmw_gid_t * gid)
{
  (void) publisher;
  (void) gid;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}
