#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>

/*
	1.0 增加版本管理
*/
static int version_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, linux_proc_banner,
		utsname()->sysname,
		utsname()->release,
		utsname()->version);
	return 0;
}

static char gUbootVer[20];

static int android_kernel_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "kernel=kernel:?,u-boot=%s",gUbootVer);
	return 0;
}

static int version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, version_proc_show, NULL);
}

static int android_kernel_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, android_kernel_proc_show, NULL);
}

static const struct file_operations version_proc_fops = {
	.open		= version_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations android_kernel_proc_fops = {
	.open		= android_kernel_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_version_init(void)
{
	proc_create("version", 0, NULL, &version_proc_fops);
	proc_create("android_kernel", 0, NULL, &android_kernel_proc_fops);
	return 0;
}

static int __init param_uboot_ver_setup(char *str)
{
 strcpy(gUbootVer,str);
 return 0;
}

module_init(proc_version_init);
__setup("my_version=", param_uboot_ver_setup);
