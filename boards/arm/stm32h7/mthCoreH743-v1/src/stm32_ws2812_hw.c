#include <nuttx/config.h>

#include <stdint.h>
#include <string.h>

#include "stm32_dma.h"
#include "stm32_tim.h"
#include "stm32_gpio.h"

#include <arch/board/board.h>
#include "mthCoreH743-v1.h"
#include <nuttx/irq.h>

/* === CONFIG === */

#define WS2812_MAX_LEDS 6

#define BITS_PER_LED 24
#define RESET_SLOTS  60

#define DUTY_0  30
#define DUTY_1  70

#define PWM_BUFFER_SIZE (WS2812_MAX_LEDS * BITS_PER_LED + RESET_SLOTS)

/* === STATIC DATA === */

static uint16_t g_pwm_buffer[PWM_BUFFER_SIZE];

/* DMA handle (you must map correct stream/channel) */
static DMA_HANDLE g_dma;

/* === GPIO + TIM INIT === */

static void ws2812_gpio_init(void)
{
  /* PA0 → TIM2_CH1 */
  stm32_configgpio(GPIO_WS2812_PWM);
}

static void ws2812_tim_init(void)
{
  /* Enable TIM2 clock etc. (board-specific) */

  putreg32(0, STM32_TIM2_CR1);

  /* Prescaler + ARR → 800kHz */
  putreg32(0, STM32_TIM2_PSC);
  putreg32(100, STM32_TIM2_ARR);

  /* PWM mode 1 */
  modifyreg32(STM32_TIM2_CCMR1, 0,
              (6 << TIM_CCMR1_OC1M_SHIFT) | TIM_CCMR1_OC1PE);

  modifyreg32(STM32_TIM2_CCER, 0, TIM_CCER_CC1E);
}

static void ws2812_dma_init(void)
{
  /* Allocate DMA channel for TIM2_CH1 */

  g_dma = stm32_dmachannel(/* YOUR DMA CHANNEL HERE */);
}

/* === DATA CONVERSION === */

static void ws2812_fill_pwm(uint8_t *data, int len)
{
  int idx = 0;

  for (int i = 0; i < len; i++)
  {
    uint8_t val = data[i];

    for (int b = 7; b >= 0; b--)
    {
      g_pwm_buffer[idx++] =
        (val & (1 << b)) ? DUTY_1 : DUTY_0;
    }
  }

  for (int i = 0; i < RESET_SLOTS; i++)
    g_pwm_buffer[idx++] = 0;
}

/* === PUBLIC API === */

int ws2812_hw_init(void)
{
  ws2812_gpio_init();
  ws2812_tim_init();
  ws2812_dma_init();

  return OK;
}

int ws2812_hw_send(uint8_t *data, int len)
{
  ws2812_fill_pwm(data, len);

  /* Clean DCache (CRITICAL for H7) */
  up_clean_dcache((uintptr_t)g_pwm_buffer,
                  sizeof(g_pwm_buffer));

  /* Start DMA transfer → TIM2 CCR1 */

  stm32_dmastart(
      g_dma,
      (uint32_t)g_pwm_buffer,
      (uint32_t)&STM32_TIM2_CCR1,
      PWM_BUFFER_SIZE);

  /* Enable DMA request */
  modifyreg32(STM32_TIM2_DIER, 0, TIM_DIER_CC1DE);

  /* Start timer */
  modifyreg32(STM32_TIM2_CR1, 0, TIM_CR1_CEN);

  return OK;
}

/* Export low-level struct */

#include <nuttx/leds/ws2812.h>

const struct ws2812_ll_s g_ws2812_ll =
{
  .init = ws2812_hw_init,
  .send = ws2812_hw_send,
};