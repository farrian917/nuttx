/****************************************************************************
 * boards/arm/stm32f0l0g0/stm32g071b-disco/src/stm32_ina226.c
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

#include <errno.h>
#include <debug.h>

#include <nuttx/i2c/i2c_master.h>


#include <nuttx/ioexpander/pca9539.h> 
#include <nuttx/ioexpander/gpio.h>    

#include "stm32_i2c.h"
#include "mthCoreH743-v1.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_STM32H7_I2C4
#  error "EXPANDER support requires CONFIG_STM32H7_I2C4"
#endif

#define EXPANDER_I2C_ADDR    0x74 /* VBUS */
// #define INA226_2_I2C_ADDR    0x41 /* CC1 */
// #define INA226_3_I2C_ADDR    0x42 /* CC2 */

#define EXPANDER_I2C_BUS       4

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_expander_initialization
 *
 * Description:
 *   Initialize and configure the INA226
 *
 ****************************************************************************/

int stm32_expander_initialization(void)
{
  struct i2c_master_s *i2c         = NULL;
  static bool          initialized = false;
  int                  ret         = OK;

  struct pca9539_config_s *pca9539_cfg = NULL; 

  FAR struct ioexpander_dev_s *ioe;

  pca9539_cfg->address = EXPANDER_I2C_ADDR;
  pca9539_cfg->frequency = 1000000;

  /* Have we already initialized? */

  if (!initialized)
    {
      /* No.. Get the I2C bus driver */

      i2c = stm32_i2cbus_initialize(EXPANDER_I2C_BUS);
      if (!i2c)
        {
          serr("ERROR: Failed to initialize I2C%d\n", EXPANDER_I2C_BUS);
          goto errout;
        }

      ioe = pca9539_initialize(i2c, pca9539_cfg);
      if (!ioe)
        {
          syslog(LOG_ERR, "ERROR: Failed to initialize PCA9539 driver\n");
          return -ENODEV;
        }

      /* Now bind the I2C interface to the EXPANDER drivers */

      ret = gpio_lower_half(ioe, 0, GPIO_OUTPUT_PIN, 0);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to register PCA9539 pin 0: %d\n", ret);
          return ret;
        }

      ret = gpio_lower_half(ioe, 15, GPIO_OUTPUT_PIN, 15);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: Failed to register PCA9539 pin 15: %d\n", ret);
          return ret;
        }



      /* Now we are initialized */

      initialized = true;
    }

errout:
  return ret;
}
