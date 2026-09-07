/****************************************************************************
 * boards/arm/stm32h7/linum-stm32h753bi/src/stm32_sdmmc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/sdio.h>
#include <nuttx/mmcsd.h>

#include "stm32_gpio.h"
#include "stm32_sdmmc.h"
#include "mthCoreH743-v1.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Card detections requires card support and a card detection GPIO */

// #define HAVE_NCD   1
// #if !defined(HAVE_SDIO) || !defined(GPIO_SDIO_NCD)
// #  undef HAVE_NCD
// #endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sdio_dev_s *g_sdio_dev;
// #ifdef HAVE_NCD
// static bool g_sd_inserted;
// #endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_ncd_interrupt
 *
 * Description:
 *   Card detect interrupt handler.
 *
 ****************************************************************************/

// #ifdef HAVE_NCD
// static int stm32_ncd_interrupt(int irq, void *context, void *param)
// {
//   bool present;

//   present = !stm32_gpioread(GPIO_SDIO_NCD);
//   if (present != g_sd_inserted)
//     {
//       sdio_mediachange(g_sdio_dev, present);
//       g_sd_inserted = present;
//     }

//   return OK;
// }
// #endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDIO-based MMC/SD card support
 *
 ****************************************************************************/

int stm32_sdio_initialize(void)
{
  int ret;

  /* Delay after board power up*/
  usleep(100000);  

  finfo("Initializing SDIO slot %d\n", SDIO_SLOTNO);

  g_sdio_dev = sdio_initialize(SDIO_SLOTNO);
  if (!g_sdio_dev)
  {
    ferr("ERROR: Failed to initialize SDIO slot %d\n", SDIO_SLOTNO);
    return -ENODEV;
  }

  sdio_mediachange(g_sdio_dev, true);

  /* Now bind the SDIO interface to the MMC/SD driver */
  finfo("Bind SDIO to the MMC/SD driver, minor=%d\n", SDIO_MINOR);

  ret = mmcsd_slotinitialize(SDIO_MINOR, g_sdio_dev);
  if (ret != OK)
    {
      ferr("ERROR: Failed to bind SDIO to the MMC/SD driver: %d\n", ret);
      return ret;
    }

  finfo("Successfully bound SDIO to the MMC/SD driver\n");

  return OK;
}
