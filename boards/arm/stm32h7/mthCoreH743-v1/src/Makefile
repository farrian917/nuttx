############################################################################
# boards/arm/stm32h7/weact-stm32h743/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(TOPDIR)/Make.defs

CSRCS = stm32_boot.c stm32_bringup.c stm32_ioctl.c

# ifeq ($(CONFIG_ARCH_LEDS),y)
# CSRCS += stm32_autoleds.c
# else
# CSRCS += stm32_userleds.c
# endif

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += stm32_appinitialize.c
endif

ifeq ($(CONFIG_STM32H7_SDMMC1),y)
CSRCS += stm32_sdmmc.c
endif

ifeq ($(CONFIG_FAT_DMAMEMORY),y)
CSRCS += stm32_dma_alloc.c
endif

ifeq ($(CONFIG_STM32H7_OTGFS),y)
CSRCS += stm32_usb.c
endif

ifeq ($(CONFIG_STM32H7_LTDC),y)
CSRCS += stm32_lcd.c
endif

ifeq ($(CONFIG_INPUT_FT5X06),y)
CSRCS += stm32_touchscreen.c
endif

ifeq ($(CONFIG_ETH0_PHY_DP83848C),y)
CSRCS += stm32_phyinit.c
endif

ifeq ($(CONFIG_SENSORS_INA226),y)
CSRCS += stm32_ina226.c
endif

ifeq ($(CONFIG_IOEXPANDER),y)
CSRCS += stm32_expander.c
endif

ifeq ($(CONFIG_WS2812),y)
CSRCS += stm32_ws2812.c
endif








include $(TOPDIR)/boards/Board.mk
