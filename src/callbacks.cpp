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

#include "./callbacks.hpp"
#include "./types.hpp"

template<typename T>
void inner_callback(
  void * callee, const rtps::ReaderCacheChange & cacheChange,
  const rmw_ertps_mempool_t * memory)
{
  rtps::Reader * reader = reinterpret_cast<rtps::Reader *>( callee);
  rmw_ertps_mempool_item_t * item = memory->allocateditems;
  while (item != NULL) {
    T * element = reinterpret_cast<T *>(item->data);

    if (element->reader == reader) {
      rmw_ertps_mempool_item_t * memory_node = get_memory(&static_buffer_memory);
      if (!memory_node) {
        RMW_SET_ERROR_MSG("Not available static buffer memory node");
        return;
      }

      rmw_ertps_static_input_buffer_t * static_buffer =
        reinterpret_cast<rmw_ertps_static_input_buffer_t *>(memory_node->data);
      static_buffer->owner = reinterpret_cast<void *>(element);
      static_buffer->length = cacheChange.getDataSize();
      static_buffer->writer_guid = cacheChange.writerGuid;
      static_buffer->sequence_number = cacheChange.sn;
      static_buffer->related_writer_guid = cacheChange.relatedWriterGuid;
      static_buffer->related_sequence_number = cacheChange.relatedSequenceNumber;
      if (!cacheChange.copyInto(static_buffer->buffer, RMW_ERTPS_MAX_INPUT_BUFFER_SIZE)) {
        put_memory(&static_buffer_memory, memory_node);
      } else {
        extern sys_sem_t rmw_wait_sem;
        sys_sem_signal(&rmw_wait_sem);
      }

      return;
    }
    item = item->next;
  }
}

template<>
void generic_callback<rmw_ertps_service_t>(
  void * callee,
  const rtps::ReaderCacheChange & cacheChange)
{
  inner_callback<rmw_ertps_service_t>(callee, cacheChange, &service_memory);
}

template<>
void generic_callback<rmw_ertps_subscription_t>(
  void * callee,
  const rtps::ReaderCacheChange & cacheChange)
{
  inner_callback<rmw_ertps_subscription_t>(callee, cacheChange, &subscription_memory);
}

template<>
void generic_callback<rmw_ertps_client_t>(
  void * callee,
  const rtps::ReaderCacheChange & cacheChange)
{
  inner_callback<rmw_ertps_client_t>(callee, cacheChange, &client_memory);
}
