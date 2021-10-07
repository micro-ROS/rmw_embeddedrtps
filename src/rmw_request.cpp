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
#include "./utils.hpp"

rmw_ret_t
rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id)
{
  if (!is_ertps_rmw_identifier_valid(client->implementation_identifier)) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  rmw_ertps_client_t * custom_client = reinterpret_cast<rmw_ertps_client_t *>(client->data);

  const rosidl_message_type_support_t * req_members =
    custom_client->type_support_callbacks->request_members_();
  const message_type_support_callbacks_t * functions =
    reinterpret_cast<const message_type_support_callbacks_t *>(req_members->data);

  static uint8_t buffer[RMW_ERTPS_MAX_OUTPUT_BUFFER_SIZE];

  // Encapsulation
  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = 0;
  buffer[3] = 0;

  ucdrBuffer mb;
  ucdr_init_buffer(&mb, &buffer[4], sizeof(buffer) - 4);

  bool written = functions->cdr_serialize(ros_request, &mb);

  if (written) {
    size_t size = ucdr_buffer_length(&mb) + 4;
    const rtps::CacheChange * cache_change = custom_client->writer->newChange(
      rtps::ChangeKind_t::ALIVE,
      buffer, size);
    *sequence_id =
      (int64_t)cache_change->sequenceNumber.high << 32 |
      (int64_t) cache_change->sequenceNumber.low;  // NO_LINT
  } else if (!written) {
    RMW_SET_ERROR_MSG("error send request");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  rmw_service_info_t * request_header,
  void * ros_request,
  bool * taken)
{
  if (taken != NULL) {
    *taken = false;
  }

  if (!is_ertps_rmw_identifier_valid(service->implementation_identifier)) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  rmw_ertps_service_t * custom_service = reinterpret_cast<rmw_ertps_service_t *>(service->data);

  // Find first related item in static buffer memory pool
  rmw_ertps_mempool_item_t * static_buffer_item =
    rmw_ertps_find_static_input_buffer_by_owner(reinterpret_cast<void *>(custom_service));
  if (static_buffer_item == NULL) {
    return RMW_RET_ERROR;
  }

  rmw_ertps_static_input_buffer_t * static_buffer =
    reinterpret_cast<rmw_ertps_static_input_buffer_t *>(static_buffer_item->data);

  std::copy(
    static_buffer->writer_guid.prefix.id.begin(),
    static_buffer->writer_guid.prefix.id.begin() + 12, request_header->request_id.writer_guid);
  std::copy(
    static_buffer->writer_guid.prefix.id.begin() + 12,
    static_buffer->writer_guid.prefix.id.begin() + 15, &request_header->request_id.writer_guid[12]);
  std::copy(
    static_buffer->writer_guid.prefix.id.begin() + 15,
    static_buffer->writer_guid.prefix.id.begin() + 16, &request_header->request_id.writer_guid[15]);
  request_header->request_id.sequence_number =
    ((int64_t) static_buffer->sequence_number.high << 32) |
    (int64_t) static_buffer->sequence_number.low;  // NO_LINT


  const rosidl_message_type_support_t * req_members =
    custom_service->type_support_callbacks->request_members_();
  const message_type_support_callbacks_t * functions =
    reinterpret_cast<const message_type_support_callbacks_t *>(req_members->data);

  ucdrBuffer temp_buffer;
  ucdr_init_buffer(
    &temp_buffer,
    &static_buffer->buffer[4],
    static_buffer->length - 4);

  bool deserialize_rv = functions->cdr_deserialize(&temp_buffer, ros_request);

  put_memory(&static_buffer_memory, static_buffer_item);

  if (taken != NULL) {
    *taken = deserialize_rv;
  }

  if (!deserialize_rv) {
    RMW_SET_ERROR_MSG("Typesupport desserialize error.");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}
