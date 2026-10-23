#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h> 
#include <linux/uaccess.h>

#define DEV_MEM_SIZE 512
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt, __func__
char device_buffer[DEV_MEM_SIZE];

dev_t device_number;

struct cdev pcd_cdev;


loff_t pcd_lseek(struct file *filp, loff_t off, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filep);
int pcd_release(struct inode *inode, struct file *filp);

struct file_operations pcd_fops ={
    .read = pcd_read,
    .write = pcd_write,
    .llseek = pcd_lseek,
    .open = pcd_open,
    .release = pcd_release,
    .owner = THIS_MODULE    
};

struct class *class_pcd;
struct device *device_pcd;


static int __init pcd_driver_init(void){

    int ret =0;
    /*dynamically allocate a device number*/
    ret = alloc_chrdev_region(&device_number,0,1,"pcd");
    if(ret < 0)
    {
        goto out;
    }

    pr_info("device number <major>:<minor> = %d:%d\n", MAJOR(device_number),MINOR(device_number));

    /*initialize the cdev structure with fops*/
    cdev_init(&pcd_cdev, &pcd_fops);
    pr_info("module cdev initialization successful");

    /*register the device (cdev structure) with VFS*/
    pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&pcd_cdev, device_number,1); //only one device-> 1
    if(ret < 0)
    {
        goto unreg_cdev;
    }
    pr_info("module registration was successful");

    /*create device class under /sys/class/*/
    class_pcd = class_create( "pcd_class" );
    if (IS_ERR(class_pcd)) {
        pr_err("class creation failed\n");
        ret = PTR_ERR(class_pcd);
        goto cdev_del;
    }
    pr_info("module class creation was successful");
    /*populate the sysfs with device information*/
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
    if (IS_ERR(device_pcd)) {
        pr_err("device_create failed\n");
        ret = PTR_ERR(device_pcd);
        goto class_del;
    }
    pr_info("module init was successful");
    return 0;
class_del:
    class_destroy(class_pcd);

cdev_del:
    cdev_del(&pcd_cdev);
    
unreg_cdev:
    unregister_chrdev_region(device_number,1);
out:
    return ret;

}

static void __exit pcd_driver_cleanup(void)
{
    device_destroy(class_pcd, device_number);
    class_destroy(class_pcd);
    cdev_del(&pcd_cdev);
    unregister_chrdev_region(device_number, 1);
    pr_info("module unloaded");
}



loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
    loff_t temp; /*long offset type*/
    pr_info("lseek requested\n");
    pr_info("current value of the file position = %lld\n", filp->f_pos);
    switch(whence)
    {
        case SEEK_SET:
            if((offset > DEV_MEM_SIZE) || (offset < 0))
            {
                return -EINVAL;
            }
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
            {
                return -EINVAL;
            }

            filp->f_pos += offset;
            break;
        case SEEK_END:
            temp = DEV_MEM_SIZE + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
            {
                return -EINVAL;
            }
            filp->f_pos  = DEV_MEM_SIZE + offset;
            break;
        default:
            return -EINVAL;
    };


    pr_info("new value of the file position = %lld\n", filp->f_pos);
    return filp->f_pos;
}


ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("read requested for %zu bytes\n", count);
    pr_info("current file position = %lld\n", *f_pos);

    /*adjust the count*/
    if((*f_pos + count) > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - *f_pos;
    }
    /*copy to user*/
    if(copy_to_user(buff, &device_buffer[*f_pos], count))
    {
        return -EFAULT;
    }

    /*update the current file position*/
    *f_pos += count;

    pr_info("number of bytes successfully read = %zu\n", count);
    pr_info("updated file position = %lld\n", *f_pos);

    /*return number of bytes successfully read*/
    return count;
}


ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("write requested for %zu bytes\n", count);
    pr_info("current file position = %lld\n", *f_pos);

    /*adjust the count*/
    if((*f_pos + count) > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - *f_pos;
    }

    if(!count)
    {
        pr_err("No space left on the driver\n");
        return -ENOMEM;
    }

    /*copy from user*/
    if(copy_from_user(&device_buffer[*f_pos], buff, count))
    {
        return -EFAULT;
    }

    /*update the current file position*/
    *f_pos += count;

    pr_info("number of bytes successfully written = %zu\n", count);
    pr_info("updated file position = %lld\n", *f_pos);

    /*return number of bytes successfully written*/
    return count;
}

int pcd_open(struct inode *inode, struct file *filp )
{
    pr_info("open was successful");
    return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
    pr_info("release was successful");
    return 0;
}



module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dulmin");
MODULE_DESCRIPTION("a pseudo character driver");