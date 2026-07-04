/****************************************************************************
 * boards/arm/stm32h7/mthCoreH743-v1/src/mthCoreH743-v1.h
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

#ifndef __BOARDS_ARM_STM32H7_MTHCOREH743_V1_SRC_MTHCOREH743_V1_H
#define __BOARDS_ARM_STM32H7_MTHCOREH743_V1_SRC_MTHCOREH743_V1_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* serial LED WS2812 PWM channel */

#define GPIO_WS2812_PWM (GPIO_TIM2_CH1OUT_1 | GPIO_SPEED_50MHz) /* PA0 TIM2_CH1 */

/* Check if we can support the RTC driver */

#define HAVE_RTC_DRIVER 1
#if !defined(CONFIG_RTC) || !defined(CONFIG_RTC_DRIVER)
#error You forgot about RTC driver
#endif

/* USB OTG FS */

#define GPIO_OTGFS_VBUS   (GPIO_INPUT | GPIO_FLOAT | GPIO_SPEED_100MHz | GPIO_OPENDRAIN | GPIO_PORTA | GPIO_PIN9) /* PA9 */

/* SD Card */

#if defined(CONFIG_STM32H7_SDMMC1) || defined(CONFIG_STM32H7_SDMMC2)
#  define HAVE_SDIO
#endif

#if defined(CONFIG_DISABLE_MOUNTPOINT) || !defined(CONFIG_MMCSD_SDIO)
#  undef HAVE_SDIO
#endif

#define SDIO_SLOTNO        0
#define SDIO_MINOR         0

/* PWM */

#define LED_PWMTIMER    2
#define BL_PWMTIMER     4

/* Ethernet control pins */

#define GPIO_ETH_NRST (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_2MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN2) /* PB2 */

/* LCD control pins */

#define GPIO_LCD_NRST   (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_2MHz | GPIO_OUTPUT_CLEAR | GPIO_PORTI | GPIO_PIN8) /* PI8 */
#define GPIO_LCD_EN     (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_2MHz | GPIO_OUTPUT_SET | GPIO_PORTK | GPIO_PIN2) /* PK2 */
#define GPIO_LCD_BL_PWM (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_2MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN7) /* PB7 TIM4_CH2_OUT1 */

/* BUTTONS */

#define GPIO_BTN_USER0  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN3)  /* PE3 */
#define GPIO_BTN_USER1  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN5)  /* PE5 */
#define GPIO_BTN_USER2  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN2)  /* PE2 */
#define GPIO_BTN_USER3  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN4)  /* PE4 */
#define GPIO_BTN_USER4  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTA | GPIO_PIN10) /* PA10 */


/****************************************************************************
 * Public Function Prototypes
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

int stm32_bringup(void);

/****************************************************************************
 * Name: stm32_usbinitialize
 *
 * Description:
 *   Called from stm32_usbinitialize very early in inialization to setup
 *   USB-related GPIO pins for the LINUM-STM32H753BI board.
 *
 ****************************************************************************/

#ifdef CONFIG_STM32H7_OTGFS
void weak_function stm32_usbinitialize(void);
#else
#error You forgot to set CONFIG_STM32H7_OTGFS
#endif

/****************************************************************************
 * Name: stm32_dma_alloc_init
 *
 * Description:
 *   Called to create a FAT DMA allocator.
 *
 * Returned Value:
 *   0 on success or -ENOMEM
 *
 ****************************************************************************/

#if defined (CONFIG_FAT_DMAMEMORY)
int stm32_dma_alloc_init(void);
#else
//#error You forgot to set CONFIG_FAT_DMAMEMORY
#endif

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDIO-based MMC/SD card support.
 *
 ****************************************************************************/

#ifdef HAVE_SDIO
int stm32_sdio_initialize(void);
#else
#error You forgot to activate SDIO/MMC 
#endif

/****************************************************************************
 * Name: stm32_pwm_setup
 *
 * Description:
 *   Initialize PWM and register the PWM device.
 *
 ****************************************************************************/

#ifdef CONFIG_PWM
int stm32_pwm_setup(void);
#else
//#error You forgot to activate PWM 
#endif

/****************************************************************************
 * Name: stm32_spidev_initialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins.
 *
 ****************************************************************************/

#ifdef CONFIG_STM32H7_SPI
void stm32_spidev_initialize(void);
#else
//#error You forgot to activate SPI 
#endif

/****************************************************************************
 * Name: stm32_n25qxxx_setup
 *
 * Description:
 *   Initialize and register the FLash for N25QXXX driver.
 *
 ****************************************************************************/

#ifdef CONFIG_MTD_W25QXXXJV
int stm32_w25qxxx_setup(void);
#else
//#error You forgot to set CONFIG_MTD_W25QXXXJV 
#endif

/****************************************************************************
 * Name: stm32_tsc_setup
 *
 * Description:
 *   This function is called by board-bringup logic to configure the
 *   touchscreen device.  This function will register the driver as
 *   /dev/inputN where N is the minor device number.
 *
 * Input Parameters:
 *   minor   - The input device minor number
 *
 * Returned Value:
 *   Zero is returned on success.  Otherwise, a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

#ifdef CONFIG_INPUT_FT5X06
int stm32_tsc_setup(int minor);
#else
//#error You forgot to set CONFIG_INPUT_FT5X06 
#endif

#endif /* __BOARDS_ARM_STM32H7_MTHCOREH743_V1_SRC_MTHCOREH743_V1_H */
