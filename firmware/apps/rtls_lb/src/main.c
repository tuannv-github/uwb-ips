/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include "os/mynewt.h"
#include "mesh/mesh.h"
#include "console/console.h"
#include "hal/hal_system.h"
#include "hal/hal_gpio.h"
#include "bsp/bsp.h"
#include "shell/shell.h"

#include "rtls_lb/ble_mesh.h"

#include <uwb/uwb.h>
#include <rtls_tdma/rtls_tdma.h>

rtls_tdma_instance_t rtls_tdma_instance;
    struct uwb_dev *udev;
int
main(int argc, char **argv)
{

    /* Initialize OS */
    sysinit();

    ble_mesh_init();

    udev = uwb_dev_idx_lookup(0);

    // uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    // printf("{\"utime\": %lu,\"exec\": \"%s\"}\n",utime,__FILE__);
    // printf("{\"device_id\"=\"%lX\"",udev->device_id);
    // printf(",\"panid=\"%X\"",udev->pan_id);
    // printf(",\"addr\"=\"%X\"",udev->uid);
    // printf(",\"part_id\"=\"%lX\"",(uint32_t)(udev->euid&0xffffffff));
    // printf(",\"lot_id\"=\"%lX\"}\n",(uint32_t)(udev->euid>>32));
    // printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime, uwb_phy_SHR_duration(udev));
    // os_time_delay(os_time_ms_to_ticks32(5000));

    rtls_tdma_start(&rtls_tdma_instance, udev);
    
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    return 0;
}
