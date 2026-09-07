/****************************************************************************
 * boards/arm/stm32h7/mthCoreH743-v1/src/stm32_ws2812.c
 ****************************************************************************/

#include <nuttx/config.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <nuttx/cache.h>
#include <nuttx/timers/pwm.h>
#include <nuttx/leds/ws2812.h> 

#include "arm_internal.h"
#include "stm32_gpio.h"
#include "hardware/stm32_pinmap.h"
#include "hardware/stm32_rcc.h"
#include "hardware/stm32_tim.h"
#include "hardware/stm32_dmamux.h"
#include "hardware/stm32_dma.h"

#ifndef CONFIG_WS2812_NON_SPI_DRIVER
#  error "This driver requires CONFIG_WS2812_NON_SPI_DRIVER=y in menuconfig"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WS2812_PWM_FREQ       800000  /* Частота ШИМ строго 800 кГц (1.25 мкс) */

/* Строго прямые (БЕЗ ИНВЕРСИЙ) значения скважности под вашу шину 240 МГц (ARR=300):
 * Логический 0 (T0H = 350 нс) -> 84 такта
 * Логическая 1 (T1H = 650 нс) -> 156 тактов
 */
#define WS2812_ARR_VALUE      (300 - 1)  
#define WS2812_CCR_BIT0       84         
#define WS2812_CCR_BIT1       156        

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct stm32_ws2812_priv_s
{
  struct ws2812_dev_s        dev;          
  uint16_t                   num_leds;     
  uint32_t                  *dma_buffer;   
  size_t                     buf_size;     
  size_t                     total_pulses; 
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int stm32_ws2812_open(FAR struct file *filep);
static int stm32_ws2812_close(FAR struct file *filep);
static ssize_t stm32_ws2812_write(FAR struct file *filep, FAR const char *data, size_t len);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32_ws2812_priv_s g_ws2812_priv;

/****************************************************************************
 * Private Data Regions
 ****************************************************************************/

/* Буфер под 100 диодов * 24 бита + 2 финишных элемента глушения ШИМ */
#define MAX_LED_PULSES ((100 * 24) + 4)
static uint32_t g_ws2812_dmabuf[MAX_LED_PULSES] locate_data(".sram1") aligned_data(32);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int stm32_ws2812_open(FAR struct file *filep)
{
  return OK;
}

static int stm32_ws2812_close(FAR struct file *filep)
{
  return OK;
}

/****************************************************************************
 * Name: stm32_ws2812_write
 ****************************************************************************/
static ssize_t stm32_ws2812_write(FAR struct file *filep, FAR const char *data, size_t len)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ws2812_dev_s *dev = inode->i_private;
  FAR struct stm32_ws2812_priv_s *priv = (FAR struct stm32_ws2812_priv_s *)dev;
  size_t num_leds_to_update;
  size_t bit_offset = 0;
  int i, bit;

  if (!data || len == 0)
    {
      return -EINVAL;
    }

  num_leds_to_update = len / 4;
  if (num_leds_to_update > priv->num_leds)
    {
      num_leds_to_update = priv->num_leds;
    }

  /* Преобразование в прямые импульсы ШИМ (PWM Mode 1) */
  for (i = 0; i < num_leds_to_update; i++)
    {
      uint8_t r = data[i * 4 + 2]; 
      uint8_t g = data[i * 4 + 1];
      uint8_t b = data[i * 4 + 0];      

      uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

      for (bit = 23; bit >= 0; bit--)
        {
          if ((grb >> bit) & 1)
            {
              priv->dma_buffer[bit_offset++] = WS2812_CCR_BIT1;
            }
          else
            {
              priv->dma_buffer[bit_offset++] = WS2812_CCR_BIT0;
            }
        }
    }

  /* ИСПРАВЛЕНО: Добавляем ДВА финишных нуля в конец транзакции DMA.
   * Благодаря включенной предварительной загрузке (Preload), первый ноль чисто 
   * завершит трансляцию 144-го бита, а второй ноль намертво защелкнет теневой 
   * регистр CCR1 в 0. Линия PA0 аппаратно упадет в идеальный LOW без иголок!
   */
  priv->dma_buffer[bit_offset++] = 0;
  priv->dma_buffer[bit_offset++] = 0;

  /* Сброс D-Cache перед аппаратным пуском DMA */
  up_clean_dcache((uintptr_t)priv->dma_buffer,
                  (uintptr_t)priv->dma_buffer + (bit_offset * sizeof(uint32_t)));

  /* === АППАРАТНАЯ НАСТРОЙКА ТАЙМЕРА И DMA1 === */
  
  /* Жестко останавливаем таймер и отключаем триггеры перед конфигурацией */
  modifyreg32(STM32_TIM2_BASE + STM32_GTIM_CR1_OFFSET, GTIM_CR1_CEN, 0);
  modifyreg32(STM32_TIM2_BASE + STM32_GTIM_DIER_OFFSET, GTIM_DIER_CC1DE, 0);

  putreg32(0, STM32_DMA1_S0CR);
  while (getreg32(STM32_DMA1_S0CR) & DMA_SCR_EN);

  /* Зачищаем флаги ошибок DMA */
  putreg32(0x3d, STM32_DMA1_BASE + STM32_DMA_LIFCR_OFFSET);
  putreg32(0x21, STM32_DMA1_BASE + STM32_DMA_S0FCR_OFFSET);

  /* Направляем PAR строго на регистр CCR1 (DBA = 13, DBL = 1 слово) */
  putreg32((uint32_t)(STM32_TIM2_BASE + STM32_GTIM_DMAR_OFFSET), STM32_DMA1_S0PAR);
  putreg32((uint32_t)priv->dma_buffer, STM32_DMA1_S0M0AR);
  putreg32(bit_offset, STM32_DMA1_S0NDTR); 

  uint32_t cr = DMA_SCR_DIR_M2P     | 
                DMA_SCR_MINC        | 
                DMA_SCR_PSIZE_32BITS | 
                DMA_SCR_MSIZE_32BITS |
                (3 << 16)           | 
                DMA_SCR_EN;
  putreg32(cr, STM32_DMA1_S0CR);

  putreg32(WS2812_ARR_VALUE, STM32_TIM2_BASE + STM32_GTIM_ARR_OFFSET);
  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_CCR1_OFFSET);
  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_CNT_OFFSET);
  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_SR_OFFSET);

  /* Включаем триггер аппаратных запросов DMA таймером */
  modifyreg32(STM32_TIM2_BASE + STM32_GTIM_DIER_OFFSET, 0, GTIM_DIER_CC1DE);

  /* Выключаем OPM — таймер работает в классическом, надежном циклическом режиме */
  modifyreg32(STM32_TIM2_BASE + STM32_GTIM_CR1_OFFSET, GTIM_CR1_OPM, 0);

  /* Запуск счетчика таймера — трансляция прямой неинвертированной пачки началась */
  modifyreg32(STM32_TIM2_BASE + STM32_GTIM_CR1_OFFSET, 0, GTIM_CR1_CEN);

  /* Никаких задержек процессора! Управление мгновенно возвращается в консоль, 
   * а линия PA0 сама аппаратно затихнет в LOW после передачи.
   */
  return len;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32_ws2812_initialize(FAR const char *devpath, uint16_t num_leds)
{
  int ret;

  syslog(LOG_INFO, "WS2812: Initializing Pure Direct NON-INVERTED PWM Mode 1 driver on PA0\n");

  modifyreg32(STM32_RCC_AHB2ENR, 0, (1 << 29) | (1 << 30));
  modifyreg32(STM32_RCC_AHB1ENR, 0, RCC_AHB1ENR_DMA1EN);
  modifyreg32(STM32_RCC_AHB4ENR, 0, (1 << 28));

  modifyreg32(STM32_RCC_APB1LENR, 0, RCC_APB1LENR_TIM2EN);
  modifyreg32(STM32_RCC_APB1LRSTR, RCC_APB1LRSTR_TIM2RST, 0);

  stm32_configgpio(GPIO_ALT | GPIO_AF1 | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN0 | GPIO_SPEED_50MHz);

  memset(&g_ws2812_priv, 0, sizeof(struct stm32_ws2812_priv_s));
  g_ws2812_priv.num_leds = num_leds;

  g_ws2812_priv.dev.open  = stm32_ws2812_open;
  g_ws2812_priv.dev.close = stm32_ws2812_close;
  g_ws2812_priv.dev.write = stm32_ws2812_write;
  g_ws2812_priv.dev.read  = NULL;
  g_ws2812_priv.dev.nleds = num_leds;

  g_ws2812_priv.total_pulses = MAX_LED_PULSES;
  g_ws2812_priv.buf_size = MAX_LED_PULSES * sizeof(uint32_t);
  g_ws2812_priv.dma_buffer = g_ws2812_dmabuf;
  memset(g_ws2812_priv.dma_buffer, 0, g_ws2812_priv.buf_size);

  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_PSC_OFFSET);
  putreg32(WS2812_ARR_VALUE, STM32_TIM2_BASE + STM32_GTIM_ARR_OFFSET);
  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_CCR1_OFFSET); 

  uint32_t dcr = (0 << 8) | 0x0D; 
  putreg32(dcr, STM32_TIM2_BASE + STM32_GTIM_DCR_OFFSET);

  /* ИСПРАВЛЕНО: Возвращаем стандартный режим ШИМ 1 (PWM Mode 1) С ВКЛЮЧЕННЫМ Preload (OC1PE=1)
   * Это зафиксирует стабильный шаг трансляции без аппаратного джиттера.
   */
  uint32_t ccmr1 = getreg32(STM32_TIM2_BASE + STM32_GTIM_CCMR1_OFFSET);
  ccmr1 &= ~GTIM_CCMR1_OC1M_MASK;
  ccmr1 |= (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR1_OC1M_SHIFT); 
  ccmr1 |= GTIM_CCMR1_OC1PE; /* Включаем Preload обратно */
  putreg32(ccmr1, STM32_TIM2_BASE + STM32_GTIM_CCMR1_OFFSET);

  /* ИСПРАВЛЕНО: Строго прямая полярность: активный ВЫСОКИЙ уровень (CC1P=0).
   * Никаких инверсий на экране осциллографа больше не будет! Сигнал идеально прямой.
   */
  uint32_t ccer = getreg32(STM32_TIM2_BASE + STM32_GTIM_CCER_OFFSET);
  ccer |= GTIM_CCER_CC1E;   
  ccer &= ~GTIM_CCER_CC1P;  /* Полярность строго прямая */
  putreg32(ccer, STM32_TIM2_BASE + STM32_GTIM_CCER_OFFSET);

  putreg32(GTIM_EGR_UG, STM32_TIM2_BASE + STM32_GTIM_EGR_OFFSET);
  putreg32(0, STM32_TIM2_BASE + STM32_GTIM_SR_OFFSET);

  uint32_t cr1 = getreg32(STM32_TIM2_BASE + STM32_GTIM_CR1_OFFSET);
  cr1 &= ~(GTIM_CR1_UDIS | GTIM_CR1_URS | GTIM_CR1_CEN); 
  putreg32(cr1, STM32_TIM2_BASE + STM32_GTIM_CR1_OFFSET);

  putreg32(18, STM32_DMAMUX1_C0CR);

  ret = ws2812_register(devpath, &g_ws2812_priv.dev);
  if (ret < 0)
    {
      return ret;
    }

  return OK;
}
