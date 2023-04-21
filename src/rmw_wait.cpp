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

#include <limits.h>
#include <math.h>

#include <rmw/rmw.h>
#include <rmw/time.h>
#include <rmw/error_handling.h>

#include <lwip/sys.h>

#include "./utils.hpp"

sys_sem_t rmw_wait_sem;

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_events_t * events,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  (void)events;
  (void)wait_set;
  (void)guard_conditions;

  // Check if timeout
  rmw_duration_t timeout = rmw_time_total_nsec(*wait_timeout);

  // Clear data flags
  bool available_data = false;
  for (size_t i = 0; !available_data && i < services->service_count; ++i) {
    rmw_ertps_service_t * custom_service =
      reinterpret_cast<rmw_ertps_service_t *>(services->services[i]);
    if (custom_service->has_data) {
      available_data = true;
    }
  }
  for (size_t i = 0; !available_data && i < clients->client_count; ++i) {
    rmw_ertps_client_t * custom_client =
      reinterpret_cast<rmw_ertps_client_t *>(clients->clients[i]);
    if (custom_client->has_data) {
      available_data = true;
    }
  }
  for (size_t i = 0; !available_data && i < subscriptions->subscriber_count; ++i) {
    rmw_ertps_subscription_t * custom_subscription =
      reinterpret_cast<rmw_ertps_subscription_t *>(subscriptions->subscribers[i]);
    if (custom_subscription->has_data) {
      available_data = true;
    }
  }

  // If there is no data and timeout is positive, wait for data
  if (!available_data && (timeout / 1000000ULL) > 0) {
    int32_t timeout_ms;
    if (rmw_time_equal(*wait_timeout, (rmw_time_t)RMW_DURATION_INFINITE)) {
      timeout_ms = static_cast<uint32_t>(0U);
    } else {
      timeout_ms = static_cast<int32_t>((timeout / 1000000ULL));
    }
    // Enable flag if sem wait is not timeout
    available_data |= (SYS_ARCH_TIMEOUT != sys_arch_sem_wait(&rmw_wait_sem, timeout_ms));
  }

  // Determine entities with data
  if (available_data) {
    // Check services
    for (size_t i = 0; i < services->service_count; ++i) {
      rmw_ertps_service_t * custom_service =
        reinterpret_cast<rmw_ertps_service_t *>(services->services[i]);

      if (NULL !=
        rmw_ertps_find_static_input_buffer_by_owner(reinterpret_cast<void *>(custom_service)))
      {
        custom_service->has_data = true;
        available_data = true;
      }
    }

    // Check clients
    for (size_t i = 0; i < clients->client_count; ++i) {
      rmw_ertps_client_t * custom_client =
        reinterpret_cast<rmw_ertps_client_t *>(clients->clients[i]);

      if (NULL !=
        rmw_ertps_find_static_input_buffer_by_owner(reinterpret_cast<void *>(custom_client)))
      {
        custom_client->has_data = true;
        available_data = true;
      }
    }

    // Check subscriptions
    for (size_t i = 0; subscriptions && i < subscriptions->subscriber_count; ++i) {
      rmw_ertps_subscription_t * custom_subscription =
        reinterpret_cast<rmw_ertps_subscription_t *>(subscriptions->subscribers[i]);

      if (NULL !=
        rmw_ertps_find_static_input_buffer_by_owner(reinterpret_cast<void *>(custom_subscription)))
      {
        custom_subscription->has_data = true;
        available_data = true;
      }
    }
  }

  // Set entities data flags
  for (size_t i = 0; i < services->service_count; ++i) {
    rmw_ertps_service_t * custom_service =
      reinterpret_cast<rmw_ertps_service_t *>(services->services[i]);
    if (!custom_service->has_data) {
      services->services[i] = NULL;
    }
  }
  for (size_t i = 0; i < clients->client_count; ++i) {
    rmw_ertps_client_t * custom_client =
      reinterpret_cast<rmw_ertps_client_t *>(clients->clients[i]);
    if (!custom_client->has_data) {
      clients->clients[i] = NULL;
    }
  }
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    rmw_ertps_subscription_t * custom_subscription =
      reinterpret_cast<rmw_ertps_subscription_t *>(subscriptions->subscribers[i]);
    if (!custom_subscription->has_data) {
      subscriptions->subscribers[i] = NULL;
    }
  }

  return (available_data) ? RMW_RET_OK : RMW_RET_TIMEOUT;
}
