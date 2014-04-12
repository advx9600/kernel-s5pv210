#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/regs-clock.h>
#include <plat/regs-timer.h>
	 
#include <mach/regs-gpio.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define DEVICE_NAME "pin_watch_dog"
#define PIN S5PV210_GPH2(1)
#define PIN_SETPIN S3C_GPIO_OUTPUT

static struct timer_list timer;
static volatile int orgin_level =0;
#define TIMER_INTERVAL (HZ*5)

static int pin_ioctl(
	struct file *file, 
	unsigned int status, 
	unsigned long cmd)
{
	printk("status:%d\n",status);
	switch(status){
		case 0:
			gpio_direction_output(PIN,0); 
			break;
		case 1:
			gpio_direction_output(PIN,1); 
			break;
	}
	return 0;
}

static void wake_up_func(unsigned long  data)
{
	if (orgin_level == 0){
		orgin_level =1;
	}else{
		orgin_level =0;
	}
	//printk("deqing watch dog %d\n",orgin_level);
	gpio_direction_output(PIN,orgin_level);
	mod_timer(&timer,jiffies+TIMER_INTERVAL);
}

static void init_pin_timer(void)
{
  timer.data = 0xff;
  timer.function = wake_up_func;
  timer.expires = jiffies + 1*HZ;
  init_timer(&timer);
}

static struct file_operations dev_fops = {
	.owner	=	THIS_MODULE,
	.unlocked_ioctl	=	pin_ioctl
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init dev_init(void)
{
	printk (DEVICE_NAME" initialized\n");
	s3c_gpio_cfgpin(PIN,PIN_SETPIN);
//	s3c_gpio_pullup(PIN,1);
	gpio_direction_output(PIN,0); // default hight level
	misc_register(&misc);
	init_pin_timer();
	mod_timer(&timer,jiffies+(HZ/10));
	return 0;
}
static void __exit dev_exit(void)
{
	misc_deregister(&misc);
}
module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
