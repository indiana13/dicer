#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>

/*
 *	Dice module is a simulator of a 6-side gaming dice roll.
 *	It uses a simple 32bit random number generator:
 *	
 *		Xi+1 = (Xi*A + B) % C
 *
 *	, where Xi is a current value, Xi+1 is the next value,
 *	A = 1664525, B = 1013904223, C = 0xFFFFFFFF
 *	All values are unsigned 32-bit numbers. Initial value is the
 *	jiffies counter value in the moment of the first roll
 *	Every subsequent value is reduced to a range of [1..6] with:
 *
 *		N = 1 + (X % 6)
 *
 *	Project goals:
 *		1. Create character devices: /dev/dicer/dice[0-3]
 *		2. Implement ioctl() call to set number of dice to roll 
 *		   (effectively means the number of sequential pseudo-random
 *		    values generated when device is read)
 *
 */

static unsigned short num_dice = 1;

struct proc_dir_entry *num_dice_p, *dicer;

static int read_proc(char *buf, char **start, off_t offset, 
	int cnt, int *eof, void *data)
{
	int len = 0;
	len = sprintf(buf+len, "Number of dice rolled: %u\n", num_dice);
	return len;
}

static int write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{	
	char line[count+1];
	unsigned short tmp_val;	
	if(copy_from_user(line, buffer, count))
		return -EFAULT;
	line[count] = 0;
	sscanf(line,"%hu", &tmp_val);
	if(tmp_val) {
		num_dice = tmp_val;
	}
	return count;
}

static int __init dicer_init(void)
{
	dicer = proc_mkdir("dicer", NULL);
	if (!dicer)
		goto error;
	num_dice_p = create_proc_read_entry("num_dice", 0, dicer, read_proc, NULL);
	if (!num_dice_p)
		goto cleanup;
	num_dice_p->write_proc = write_proc;
	return 0;
cleanup:
	remove_proc_entry(dicer->name, NULL);
error:
	printk(KERN_ALERT "DICE: Failed to create /proc entries\n");
	return -1;
}

static void __exit dicer_exit(void)
{
	remove_proc_entry(num_dice_p->name, dicer);
	remove_proc_entry(dicer->name, NULL);
}

module_param(num_dice, ushort, S_IRUGO | S_IWUSR);
module_init(dicer_init);
module_exit(dicer_exit);

MODULE_LICENSE("Dual BSD/GPL");

