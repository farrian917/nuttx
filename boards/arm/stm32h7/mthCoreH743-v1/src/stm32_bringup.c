/****************************************************************************
 * boards/arm/stm32h7/mthCoreH743-v1/src/stm32_bringup.c
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

#include <sys/types.h>
#include <syslog.h>
#include <errno.h>

#include <arch/board/board.h>

#include <nuttx/fs/fs.h>

#include "mthCoreH743-v1.h"

#include "stm32_gpio.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=n && CONFIG_BOARDCTL=y &&
 *   CONFIG_NSH_ARCHINIT:
 *     Called from the NSH library
 *
 ****************************************************************************/

int stm32_bringup(void)
{
  int ret = OK;

  UNUSED(ret);

  
  // stm32_configgpio(GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN8); /* PG3 */

  // for(int iii = 0; iii < 1000; iii++)
  // {
  //  stm32_gpiowrite(GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN8, 0);

  //   usleep(10000);

  //    stm32_gpiowrite(GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN8, 1);
  //     usleep(10000);
  // }



  stm32_configgpio(GPIO_SDMMC1_D1_FLOAT);
  stm32_configgpio(GPIO_SDMMC1_D2_FLOAT);
  stm32_configgpio(GPIO_SDMMC1_D3_FLOAT);  
  stm32_configgpio(GPIO_SDMMC1_D4_FLOAT);
  stm32_configgpio(GPIO_SDMMC1_D5_FLOAT);
  stm32_configgpio(GPIO_SDMMC1_D6_FLOAT);
  stm32_configgpio(GPIO_SDMMC1_D7_FLOAT);

  stm32_configgpio(GPIO_LCD_EN);
  stm32_configgpio(GPIO_ETH_NRST);
  stm32_configgpio(GPIO_LCD_BL_PWM);

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: Failed to mount the PROC filesystem: %d\n",  ret);
    }
#endif /* CONFIG_FS_PROCFS */

#if defined(CONFIG_FAT_DMAMEMORY)
  if (stm32_dma_alloc_init() < 0)
    {
      syslog(LOG_ERR, "DMA alloc FAILED");
    }
#endif

#if defined(HAVE_SDIO)
  /* Initialize the SDIO block driver */

  ret = stm32_sdio_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR,
             "ERROR: Failed to initialize MMC/SD driver: %d\n", ret);
    }

  // ret = mkfatfs("/dev/mmcsd0", NULL);

  // if (ret < 0)
  // {
  //   syslog(LOG_ERR, "ERROR: Failed to make FATFS on drive: %d\n", ret);
  // }

#endif

  return OK;
}
