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
#include <nuttx/sensors/ina226.h>

#include "stm32_i2c.h"
#include "mthCoreH743-v1.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_STM32H7_I2C4
#  error "INA226 support requires CONFIG_STM32H7_I2C4"
#endif

#define INA226_I2C_ADDR    0x40 /* VBUS */
// #define INA226_2_I2C_ADDR    0x41 /* CC1 */
// #define INA226_3_I2C_ADDR    0x42 /* CC2 */

#define INA226_I2C_BUS       4
#define INA226_SHUNT_VAL     2000

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_ina226_initialization
 *
 * Description:
 *   Initialize and configure the INA226
 *
 ****************************************************************************/

int stm32_ina226_initialization(void)
{
  struct i2c_master_s *i2c         = NULL;
  static bool          initialized = false;
  int                  ret         = OK;

  /* Have we already initialized? */

  if (!initialized)
    {
      /* No.. Get the I2C bus driver */

      i2c = stm32_i2cbus_initialize(INA226_I2C_BUS);
      if (!i2c)
        {
          serr("ERROR: Failed to initialize I2C%d\n", INA226_I2C_BUS);
          goto errout;
        }

      /* Now bind the I2C interface to the INA226 drivers */

      ret = ina226_register("/dev/ina226", i2c, INA226_I2C_ADDR, INA226_SHUNT_VAL, 0);
      if (ret < 0)
        {
          serr("ERROR: Failed to bind I2C%d to the INA226 driver\n", INA226_I2C_BUS);
          goto errout;
        }

      /* Now we are initialized */

      initialized = true;
    }

errout:
  return ret;
}
