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

#include <rtps/rtps.h>

#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./types.hpp"
#include "./utils.hpp"

rmw_ret_t
rmw_publish(
  const rmw_publisher_t * publisher,
  const void * ros_message,
  rmw_publisher_allocation_t * allocation)
{
  (void)allocation;
  rmw_ret_t ret = RMW_RET_OK;
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher pointer is null");
    ret = RMW_RET_ERROR;
  } else if (!ros_message) {
    RMW_SET_ERROR_MSG("ros_message pointer is null");
    ret = RMW_RET_ERROR;
  } else if (!is_ertps_rmw_identifier_valid(publisher->implementation_identifier)) {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    ret = RMW_RET_ERROR;
  } else {
    rmw_ertps_publisher_t * custom_publisher =
      reinterpret_cast<rmw_ertps_publisher_t *>(publisher->data);
    const message_type_support_callbacks_t * functions = custom_publisher->type_support_callbacks;

    // TODO(pablogs): Consider using a different allocator for the message.
    static uint8_t buffer[RMW_ERTPS_MAX_OUTPUT_BUFFER_SIZE];

    // TODO(pablogs): Consider refactoring CDR encapsulation
    // Encapsulation
    buffer[0] = 0;
    buffer[1] = 1;
    buffer[2] = 0;
    buffer[3] = 0;

    ucdrBuffer mb;
    ucdr_init_buffer(&mb, &buffer[4], sizeof(buffer) - 4);

    bool written = functions->cdr_serialize(ros_message, &mb);

    if (written) {
      size_t size = ucdr_buffer_length(&mb) + 4;
      custom_publisher->writer->newChange(
        rtps::ChangeKind_t::ALIVE,
        buffer, size);
    } else if (!written) {
      RMW_SET_ERROR_MSG("error publishing message");
      ret = RMW_RET_ERROR;
    }
  }
  return ret;
}

rmw_ret_t
rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message,
  rmw_publisher_allocation_t * allocation)
{
  (void)publisher;
  (void)serialized_message;
  (void)allocation;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_publish_loaned_message(
  const rmw_publisher_t * publisher,
  void * ros_message,
  rmw_publisher_allocation_t * allocation)
{
  (void)publisher;
  (void)ros_message;
  (void)allocation;

  RMW_SET_ERROR_MSG("Function not implemented");
  return RMW_RET_UNSUPPORTED;
}
