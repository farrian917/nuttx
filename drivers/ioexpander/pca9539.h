/****************************************************************************
 * drivers/ioexpander/pca9539.h
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

#ifndef __DRIVERS_IOEXPANDER_PCA9539_H
#define __DRIVERS_IOEXPANDER_PCA9539_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/wdog.h>
#include <nuttx/clock.h>
#include <nuttx/mutex.h>
#include <nuttx/wqueue.h>
#include <nuttx/ioexpander/ioexpander.h>
#include <nuttx/ioexpander/pca9539.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/irq.h>

#if defined(CONFIG_IOEXPANDER) && defined(CONFIG_IOEXPANDER_PCA9539)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Prerequisites:
 *   CONFIG_I2C
 *     I2C support is required
 *   CONFIG_IOEXPANDER
 *     Enables support for the PCA9539 I/O expander
 *
 * CONFIG_IOEXPANDER_PCA9539
 *   Enables support for the PCA9539 driver (Needs CONFIG_INPUT)
 * CONFIG_PCA9539_MULTIPLE
 *   Can be defined to support multiple PCA9539 devices on board.
 * CONFIG_PCA9539_INT_NCALLBACKS
 *   Maximum number of supported pin interrupt callbacks.
 */

#ifdef CONFIG_IOEXPANDER_INT_ENABLE
#  ifndef CONFIG_PCA9539_INT_NCALLBACKS
#    define CONFIG_PCA9539_INT_NCALLBACKS 4
#  endif
#endif

#ifdef CONFIG_IOEXPANDER_INT_ENABLE
#  ifndef CONFIG_SCHED_WORKQUEUE
#    error Work queue support required.  CONFIG_SCHED_WORKQUEUE must be selected.
#  endif
#endif

#undef CONFIG_PCA9539_REFCNT

/* PCA9539 Resources ********************************************************/

#define PCA9539_GPIO_NPINS 16

#ifndef CONFIG_I2C
#error "CONFIG_I2C is required by PCA9539"
#endif

#define PCA9539_MAXDEVS             4

/* I2C frequency */

#define PCA9539_I2C_MAXFREQUENCY    400000       /* 400KHz */

/* PCA9539 Registers ********************************************************/

#define PCA9539_REG_INPUT0  0x00
#define PCA9539_REG_INPUT1  0x01
#define PCA9539_REG_OUTPUT0 0x02
#define PCA9539_REG_OUTPUT1 0x03
#define PCA9539_REG_POLINV0 0x04
#define PCA9539_REG_POLINV1 0x05
#define PCA9539_REG_CONFIG0 0x06
#define PCA9539_REG_CONFIG1 0x07

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifdef CONFIG_IOEXPANDER_INT_ENABLE
/* This type represents on registered pin interrupt callback */

struct PCA9539_callback_s
{
  ioe_pinset_t pinset;        /* Set of pin interrupts that will generate
                                * the callback. */
  ioe_callback_t cbfunc;      /* The saved callback function pointer */
  FAR void *cbarg;            /* Callback argument */
};
#endif

/* This structure represents the state of the PCA9539 driver */

struct pca9539_dev_s
{
  struct ioexpander_dev_s      dev;      /* Nested structure to allow casting
                                          * as public gpio expander. */
#ifdef CONFIG_PCA9539_SHADOW_MODE
  uint8_t sreg[16];                       /* Shadowed registers of the PCA9539 */
#endif
#ifdef CONFIG_PCA9539_MULTIPLE
  FAR struct PCA9539_dev_s    *flink;    /* Supports a singly linked list of drivers */
#endif
  FAR struct pca9539_config_s *config;   /* Board configuration data */
  FAR struct i2c_master_s     *i2c;      /* Saved I2C driver instance */
  mutex_t                      lock;     /* Mutual exclusion */

#ifdef CONFIG_IOEXPANDER_INT_ENABLE
  struct work_s work;                    /* Supports the interrupt handling "bottom half" */

  /* Saved callback information for each I/O expander client */

  struct PCA9539_callback_s cb[CONFIG_PCA9539_INT_NCALLBACKS];
#endif
};

#endif /* CONFIG_IOEXPANDER && CONFIG_IOEXPANDER_PCA9539 */
#endif /* __DRIVERS_IOEXPANDER_PCA9539_H */
