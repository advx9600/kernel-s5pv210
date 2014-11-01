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

#define DEVICE_NAME "calibrate-a"

#define print(...) printk(DEVICE_NAME":"__VA_ARGS__)

static struct ts_calibration {
int a [10];
} cal;

static int isNeedCalibrate=0;

typedef struct write_a{
#define OPS_WRITE_CALIBRATE_DATA 1
#define OPS_CALIBRATE_PRINT_MSG 2
        int ops;
	int val[10];
	char buf[200];
}Write_a;

int ts_linear_scale (int * x, int * y, int max_x,int max_y)
{
	int xtemp, ytemp;

	xtemp = *x;
	ytemp = *y;

	if (cal.a [6] == 0)
		return-EINVAL;

	*x = (cal.a [2] + cal.a [0] * xtemp + cal.a [1] * ytemp) / cal.a [6];
	*y = (cal.a [5] + cal.a [3] * xtemp + cal.a [4] * ytemp) / cal.a [6];

	*x = *x * max_x/cal.a[7];
	*y = *y * max_y/cal.a[8];
	return 0;
}

EXPORT_SYMBOL(ts_linear_scale);

void set_my_calibrate(int isCalibrate)
{
	isNeedCalibrate=isCalibrate;
}

EXPORT_SYMBOL(set_my_calibrate);


static ssize_t fs_write (struct file * a, const char * data, size_t size, loff_t * b)
{
	unsigned long copy=0;
	int i;
	char buf[1024];
	Write_a* wrA;
	if (size !=sizeof(Write_a)){
	  print("write size error\n");
	  return -1;
	}
	copy = copy_from_user(buf, data, size);  
	wrA=(Write_a*)buf;
	if (copy){
	  print("copy failed!\n");
	  return -1;
	}
	if (wrA->ops == OPS_WRITE_CALIBRATE_DATA){
		for (i=0;i<9;i++){
			cal.a[i]=wrA->val[i];
			print("cal.a[%d]:%d\n",i,cal.a[i]);
		}
	}else if (wrA->ops == OPS_CALIBRATE_PRINT_MSG){
		print("%s",wrA->buf);
	}

	return size;
}

ssize_t fs_read(struct file *file, char __user *user, size_t size, loff_t * loff)
{
	if (copy_to_user(user,&isNeedCalibrate,sizeof(int))){
		return -1;
	}
	return sizeof(int);
}

static struct file_operations dev_fops = {
	.owner	=	THIS_MODULE,
	.write = fs_write,
	.read = fs_read,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

#if 0
static irqreturn_t s3c_keypad_isr(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}
#endif

static int __init dev_init(void)
{
	int ret =0;
	print ("initialized\n");
	for(ret =0;ret<7;ret++)
		cal.a[ret]=0;
	ret=misc_register(&misc);
	return ret;
}

static void __exit dev_exit(void)
{
	print("removed\n");
	misc_deregister(&misc);
}
module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
