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

#include "./memory.hpp"

#include <string.h>
#include <rmw/allocators.h>


static bool has_memory(
  rmw_ertps_mempool_t * mem)
{
  bool rv = mem->freeitems != NULL ? true : false;

  return rv;
}

rmw_ertps_mempool_item_t * get_memory(
  rmw_ertps_mempool_t * mem)
{
  rtps::Lock lock{mem->memory_mutex};

  rmw_ertps_mempool_item_t * item = NULL;

  if (has_memory(mem)) {
    // Gets item from free pool
    item = mem->freeitems;
    mem->freeitems = item->next;
    if (mem->freeitems) {
      mem->freeitems->prev = NULL;
    }

    // Puts item in allocated pool
    item->next = mem->allocateditems;
    if (item->next) {
      item->next->prev = item;
    }
    item->prev = NULL;
    mem->allocateditems = item;
  } else if (mem->is_dynamic_allowed) {
#ifdef RMW_ERTPS_ALLOW_DYNAMIC_ALLOCATIONS
    item =
      reinterpret_cast<rmw_ertps_mempool_item_t *>(rmw_allocate(sizeof(rmw_ertps_mempool_item_t)));
    item->prev = NULL;
    item->next = NULL;
    item->data = reinterpret_cast<void *>(rmw_allocate(mem->element_size));
    memset(item->data, 0, mem->element_size);
    item->is_dynamic_memory = false;     // Allow to put element in free pool the first time
    put_memory(mem, item);
    item->is_dynamic_memory = true;
    item = get_memory(mem);
#endif /* ifdef RMW_ERTPS_ALLOW_DYNAMIC_ALLOCATIONS */
  }

  return item;
}

void put_memory(
  rmw_ertps_mempool_t * mem,
  rmw_ertps_mempool_item_t * item)
{
  rtps::Lock lock{mem->memory_mutex};

  // Gets item from allocated pool
  if (item->prev) {
    item->prev->next = item->next;
  }
  if (item->next) {
    item->next->prev = item->prev;
  }

  if (mem->allocateditems == item) {
    mem->allocateditems = item->next;
  }

#ifdef RMW_ERTPS_ALLOW_DYNAMIC_ALLOCATIONS
  if (item->is_dynamic_memory) {
    rmw_free(item->data);
    rmw_free(item);
    return;
  }
#endif /* ifdef RMW_ERTPS_ALLOW_DYNAMIC_ALLOCATIONS */

  // Puts item in free pool
  item->next = mem->freeitems;
  if (item->next) {
    item->next->prev = item;
  }
  item->prev = NULL;
  mem->freeitems = item;
}
