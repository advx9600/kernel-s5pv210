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

#define DEVICE_NAME "pin-485"
#define PIN485_1 S5PV210_GPH2(0)
#define PIN485_1_SETPIN S3C_GPIO_OUTPUT
#define UTRSTAT1 0xE2900010

static volatile unsigned int* Pin1;

static int pin485_ioctl(
	struct file *file, 
	unsigned int status, 
	unsigned long cmd)
{
	printk("status:%d\n",status);
	switch(status){
		case 0:
			while(!((*Pin1) &0x4));
			gpio_direction_output(PIN485_1,0); 
			break;
		case 1:
			while(!((*Pin1) &0x4));
			gpio_direction_output(PIN485_1,1); 
			break;
	}
	return 0;
}

static struct file_operations dev_fops = {
	.owner	=	THIS_MODULE,
	.unlocked_ioctl	=	pin485_ioctl
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init dev_init(void)
{
	printk (DEVICE_NAME" initialized\n");
	s3c_gpio_cfgpin(PIN485_1,PIN485_1_SETPIN);
//	s3c_gpio_pullup(PIN485_1,1);
	gpio_direction_output(PIN485_1,0); // default hight level
	misc_register(&misc);
	Pin1=ioremap(UTRSTAT1,4);		// UTRSTAT1
	printk(DEVICE_NAME" after ioremap\n");
	return 0;
}
static void __exit dev_exit(void)
{
	misc_deregister(&misc);
	iounmap(Pin1);
}
module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
