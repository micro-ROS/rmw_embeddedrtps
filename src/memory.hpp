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

#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include <stdbool.h>
#include <stddef.h>
#include "rtps/utils/Lock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rmw_ertps_mempool_item_t
{
  struct rmw_ertps_mempool_item_t * prev;
  struct rmw_ertps_mempool_item_t * next;
  void * data;
  bool is_dynamic_memory;
} rmw_ertps_mempool_item_t;

typedef struct rmw_ertps_mempool_t
{
  sys_mutex_t memory_mutex;

  struct rmw_ertps_mempool_item_t * allocateditems;
  struct rmw_ertps_mempool_item_t * freeitems;

  size_t element_size;
  bool is_initialized;
  bool is_dynamic_allowed;
} rmw_ertps_mempool_t;

rmw_ertps_mempool_item_t * get_memory(
  rmw_ertps_mempool_t * mem);
void put_memory(
  rmw_ertps_mempool_t * mem,
  rmw_ertps_mempool_item_t * item);

#ifdef __cplusplus
}
#endif

#endif  // MEMORY_HPP_
