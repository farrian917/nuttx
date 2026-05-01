#pragma once

#include <stdint.h>
#include <nuttx/fs/ioctl.h>

/* IOCTL commands */

#define WS2812IOC_SETALL   _IOW('W', 1, uint8_t *)
#define WS2812IOC_SHOW     _IO('W', 2)
#define WS2812IOC_CLEAR    _IO('W', 3)

/* Configuration */

struct ws2812_config_s
{
  uint16_t nleds;
};

/* Low-level interface (implemented in STM32 backend) */

struct ws2812_ll_s
{
  int (*init)(void);
  int (*send)(uint8_t *data, int len);
};

/* Driver registration */

int ws2812_register(const char *path,
                    const struct ws2812_ll_s *ll,
                    const struct ws2812_config_s *cfg);