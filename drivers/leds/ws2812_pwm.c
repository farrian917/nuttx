#include <nuttx/config.h>

#include <nuttx/fs/fs.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mutex.h>
#include <nuttx/semaphore.h>
#include <nuttx/leds/ws2812.h>

#include <string.h>
#include <errno.h>

struct ws2812_dev_s
{
  mutex_t lock;
  uint16_t nleds;
  uint8_t *buffer;
  const struct ws2812_ll_s *ll;
};

/* File operations */

static int ws2812_open(struct file *filep)
{
  return OK;
}

static int ws2812_close(struct file *filep)
{
  return OK;
}

static ssize_t ws2812_write(struct file *filep,
                           const char *buffer,
                           size_t len)
{
  struct ws2812_dev_s *dev = filep->f_inode->i_private;
  size_t expected = dev->nleds * 3;

  if (len != expected)
    return -EINVAL;

  nxmutex_lock(&dev->lock);
  memcpy(dev->buffer, buffer, len);
  nxmutex_unlock(&dev->lock);

  return len;
}

static int ws2812_ioctl(struct file *filep,
                        int cmd,
                        unsigned long arg)
{
  struct ws2812_dev_s *dev = filep->f_inode->i_private;
  int ret = OK;

  nxmutex_lock(&dev->lock);

  switch (cmd)
  {
    case WS2812IOC_SETALL:
      memcpy(dev->buffer, (void *)arg, dev->nleds * 3);
      break;

    case WS2812IOC_CLEAR:
      memset(dev->buffer, 0, dev->nleds * 3);
      break;

    case WS2812IOC_SHOW:
      ret = dev->ll->send(dev->buffer, dev->nleds * 3);
      break;

    default:
      ret = -ENOTTY;
      break;
  }

  nxmutex_unlock(&dev->lock);
  return ret;
}

static const struct file_operations g_ws2812_fops =
{
  .open  = ws2812_open,
  .close = ws2812_close,
  .read  = NULL,
  .write = ws2812_write,
  .ioctl = ws2812_ioctl,
};

/* Registration */

int ws2812_register(const char *path,
                    const struct ws2812_ll_s *ll,
                    const struct ws2812_config_s *cfg)
{
  struct ws2812_dev_s *dev;

  dev = kmm_zalloc(sizeof(*dev));
  if (!dev)
    return -ENOMEM;

  dev->nleds = cfg->nleds;
  dev->ll    = ll;

  nxmutex_init(&dev->lock);

  dev->buffer = kmm_zalloc(cfg->nleds * 3);
  if (!dev->buffer)
    return -ENOMEM;

  if (ll->init)
    ll->init();

  return register_driver(path, &g_ws2812_fops, 0666, dev);
}