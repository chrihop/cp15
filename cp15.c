/**
 * @file cp15.c
 * @author Hao (chrihop@gmail.com)
 * @brief cp15 driver
 * @version 0.1
 * @date 2019-08-28
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>

#include "cp15.h"

static int major = -1;
static struct cdev cp15_cdev;
static struct class *cp15_dev_class = NULL;
static struct device *cp15_device = NULL;

/*
 * B3.18.1: Identification registers, functional group
 */

#define op(i)				#i
#define c(i)				"c"#i
#define cp15_opcode(op0, cn, cm, op1)	"p15, "op(op0)", %0, "c(cn)", "c(cm)", "op(op1)
#define mrc_asm(reg, op0, cn, cm, op1)	"mrc "cp15_opcode(op0, cn, cm, op1) : "=r"(reg)
#define mrc(reg, op0, cn, cm, op1)	asm volatile(mrc_asm(reg, op0, cn, cm, op1))

#define mrc_define(op0, cn, cm, op1)	\
	static unsigned int mrc_##op0##_##cn##_##cm##_##op1(void)	\
	{							\
		register unsigned int reg;			\
		mrc(reg, op0, cn, cm, op1);			\
		return (reg);					\
	}

/* define all (16348) mrc functions */

/* definition of op0: 0~7 */
#define mrc_define_op0(cn, cm, op1)	mrc_define(0, cn, cm, op1)	\
					mrc_define(1, cn, cm, op1)	\
					mrc_define(2, cn, cm, op1)	\
					mrc_define(3, cn, cm, op1)	\
					mrc_define(4, cn, cm, op1)	\
					mrc_define(5, cn, cm, op1)	\
					mrc_define(6, cn, cm, op1)	\
					mrc_define(7, cn, cm, op1)

/* definition of cn: 0~15 */
#define mrc_define_cn(cm, op1)		mrc_define_op0(0, cm, op1)	\
					mrc_define_op0(1, cm, op1)	\
					mrc_define_op0(2, cm, op1)	\
					mrc_define_op0(3, cm, op1)	\
					mrc_define_op0(4, cm, op1)	\
					mrc_define_op0(5, cm, op1)	\
					mrc_define_op0(6, cm, op1)	\
					mrc_define_op0(7, cm, op1)	\
					mrc_define_op0(8, cm, op1)	\
					mrc_define_op0(9, cm, op1)	\
					mrc_define_op0(10, cm, op1)	\
					mrc_define_op0(11, cm, op1)	\
					mrc_define_op0(12, cm, op1)	\
					mrc_define_op0(13, cm, op1)	\
					mrc_define_op0(14, cm, op1)	\
					mrc_define_op0(15, cm, op1)

/* definition of cm: 0~15 */
#define mrc_define_cm(op1)		mrc_define_cn( 0, op1)		\
					mrc_define_cn( 1, op1)		\
					mrc_define_cn( 2, op1)		\
					mrc_define_cn( 3, op1)		\
					mrc_define_cn( 4, op1)		\
					mrc_define_cn( 5, op1)		\
					mrc_define_cn( 6, op1)		\
					mrc_define_cn( 7, op1)		\
					mrc_define_cn( 8, op1)		\
					mrc_define_cn( 9, op1)		\
					mrc_define_cn(10, op1)		\
					mrc_define_cn(11, op1)		\
					mrc_define_cn(12, op1)		\
					mrc_define_cn(13, op1)		\
					mrc_define_cn(14, op1)		\
					mrc_define_cn(15, op1)

/* definition of op1: 0~7 */
#define mrc_define_op1()		mrc_define_cm(0)	\
					mrc_define_cm(1)	\
					mrc_define_cm(2)	\
					mrc_define_cm(3)	\
					mrc_define_cm(4)	\
					mrc_define_cm(5)	\
					mrc_define_cm(6)	\
					mrc_define_cm(7)

#define mrc_define_all			mrc_define_op1()

/* define all cp15 access functions */
mrc_define_all

#define mrc_refer(op0, cn, cm, op1)	\
	mrc_##op0##_##cn##_##cm##_##op1

#define mrc_refer_op0(cn, cm, op1)	{				\
					mrc_refer(0, cn, cm, op1),	\
					mrc_refer(1, cn, cm, op1),	\
					mrc_refer(2, cn, cm, op1),	\
					mrc_refer(3, cn, cm, op1),	\
					mrc_refer(4, cn, cm, op1),	\
					mrc_refer(5, cn, cm, op1),	\
					mrc_refer(6, cn, cm, op1),	\
					mrc_refer(7, cn, cm, op1),	\
					}

#define mrc_refer_cn(cm, op1)		{				\
					mrc_refer_op0( 0, cm, op1),	\
					mrc_refer_op0( 1, cm, op1),	\
					mrc_refer_op0( 2, cm, op1),	\
					mrc_refer_op0( 3, cm, op1),	\
					mrc_refer_op0( 4, cm, op1),	\
					mrc_refer_op0( 5, cm, op1),	\
					mrc_refer_op0( 6, cm, op1),	\
					mrc_refer_op0( 7, cm, op1),	\
					mrc_refer_op0( 8, cm, op1),	\
					mrc_refer_op0( 9, cm, op1),	\
					mrc_refer_op0(10, cm, op1),	\
					mrc_refer_op0(11, cm, op1),	\
					mrc_refer_op0(12, cm, op1),	\
					mrc_refer_op0(13, cm, op1),	\
					mrc_refer_op0(14, cm, op1),	\
					mrc_refer_op0(15, cm, op1),	\
					}

#define mrc_refer_cm(op1)		{				\
					mrc_refer_cn( 0, op1),		\
					mrc_refer_cn( 1, op1),		\
					mrc_refer_cn( 2, op1),		\
					mrc_refer_cn( 3, op1),		\
					mrc_refer_cn( 4, op1),		\
					mrc_refer_cn( 5, op1),		\
					mrc_refer_cn( 6, op1),		\
					mrc_refer_cn( 7, op1),		\
					mrc_refer_cn( 8, op1),		\
					mrc_refer_cn( 9, op1),		\
					mrc_refer_cn(10, op1),		\
					mrc_refer_cn(11, op1),		\
					mrc_refer_cn(12, op1),		\
					mrc_refer_cn(13, op1),		\
					mrc_refer_cn(14, op1),		\
					mrc_refer_cn(15, op1),		\
					}

#define mrc_refer_op1()			{			\
					mrc_refer_cm(0),	\
					mrc_refer_cm(1),	\
					mrc_refer_cm(2),	\
					mrc_refer_cm(3),	\
					mrc_refer_cm(4),	\
					mrc_refer_cm(5),	\
					mrc_refer_cm(6),	\
					mrc_refer_cm(7),	\
					}

#define mrc_refer_all			mrc_refer_op1()

typedef unsigned int (*mrc_fp_t)(void);

#define N_OP0		( 8u)
#define N_OP1		( 8u)
#define N_CN		(16u)
#define N_CM		(16u)

static mrc_fp_t mrc_fp[N_OP1][N_CM][N_CN][N_OP0] = mrc_refer_all;

static unsigned int mrc_call(unsigned char op0, unsigned char cn,
				unsigned char cm, unsigned char op1)
{
	return (mrc_fp[op1][cm][cn][op0]());
}

static int cp15_open(struct inode* inode, struct file* file)
{
	return 0;
}

static int cp15_close(struct inode* inode, struct file* file)
{
	return 0;
}

static struct cp15_request_t  cp15_request;
static struct cp15_response_t cp15_response;

static ssize_t cp15_read(struct file* file, char *buf,
			 size_t count, loff_t *f_pos)
{
	register unsigned int rv;

	if (buf == NULL || count != SIZEOF_RESPONSE)
	{
		return 0;
	}

	if (cp15_response.status != CP15_OK)
	{
		memcpy(buf, &cp15_response, SIZEOF_RESPONSE);
		return (SIZEOF_RESPONSE);
	}

	rv = mrc_call(cp15_request.op0, cp15_request.cn, cp15_request.cm, cp15_request.op1);
	cp15_response.status = CP15_OK;
	cp15_response.result = rv;
	memcpy(buf, &cp15_response, SIZEOF_RESPONSE);

	printk(KERN_INFO "cp15: mrc p15, %d, x, c%d, c%d, %d => 0x%08x.\n",
		cp15_request.op0, cp15_request.cn, cp15_request.cm, cp15_request.op1, rv);

	return (SIZEOF_RESPONSE);
}

static ssize_t cp15_write(struct file* file, const char *buf,
			  size_t count, loff_t *f_pos)
{
	if (buf == NULL || count != SIZEOF_REQUEST)
	{
		cp15_response.status = CP15_FAIL;
		return 0;
	}

	memcpy(&cp15_request, buf, SIZEOF_RESPONSE);
	cp15_response.status = CP15_OK;

	return SIZEOF_REQUEST;
}

static struct file_operations dev_cp15_fops =
{
	owner:		 THIS_MODULE,
	open:		 cp15_open,
	release:	 cp15_close,
	read:		 cp15_read,
	write:		 cp15_write,
};

static void cleanup(int is_device_created)
{
	if (is_device_created)
	{
		device_destroy(cp15_dev_class, major);
		cdev_del(&cp15_cdev);
	}
	if (cp15_dev_class)
		class_destroy(cp15_dev_class);
	if (major != -1)
		unregister_chrdev_region(major, 1);
}

/* grant user permission to /dev/cp15 */
static char *cp15_devnode(struct device *dev, umode_t *mode)
{
	if (!mode)
	{
		return NULL;
	}
	*mode = 0666;
	return NULL;
}

static int __init init_cp15(void)
{
	int is_cp15_dev_created = 0;
	printk(KERN_INFO "cp15: started ...\n");

	if (alloc_chrdev_region(&major, 0, 1, DEV) < 0)
	{
		goto error;
	}
	if ((cp15_dev_class = class_create(THIS_MODULE, DEV)) == NULL)
	{
		goto error;
	}
	cp15_dev_class->devnode = cp15_devnode;
	if ((cp15_device = device_create(cp15_dev_class, NULL, major, NULL, DEV)) == NULL)
	{
		goto error;
	}
	is_cp15_dev_created = 1;
	cdev_init(&cp15_cdev, &dev_cp15_fops);
	if (cdev_add(&cp15_cdev, major, 1) == -1)
	{
		goto error;
	}
	return 0;

error:
	printk(KERN_INFO "cp15: '/dev/cp15' creation failed.\n");
	cleanup(is_cp15_dev_created);
	return -1;

}

static void __exit exit_cp15(void)
{
	cleanup(1);
	printk(KERN_INFO "cp15: exit ...\n");
}

module_init(init_cp15);
module_exit(exit_cp15);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hao");
MODULE_DESCRIPTION("cp15, an ARM cp15 probe");
