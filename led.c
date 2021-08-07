#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/io.h>

#define LED_MAJOR       200
#define LED_NAME        "led"

/* 寄存器物理地址 */
#define CCM_CCGR1_BASE          (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE  (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE  (0X020E02F4)
#define GPIO1_DR_BASE           (0X0209C000)
#define GPIO1_DIR_BASE          (0X0209C004)

#define LED_ON          1
#define LED_OFF         0

/* 映射后的虚拟地址 */
static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_DIR;

static int led_open(struct inode *inode, struct file *file);
static ssize_t led_read(struct file *file, char __user *user, size_t size, loff_t *loff);
static ssize_t led_write(struct file *file, const char __user *user, size_t size, loff_t *loff);
static int led_release(struct inode *inode, struct file *file);
static void led_switch(unsigned char led_status);
static void led_gpio_init(void);

static const struct file_operations led_fops = {
        .owner = THIS_MODULE,
        .open = led_open,
        .read = led_read,
        .write = led_write,
        .release = led_release,
};

static void led_switch(unsigned char led_status)
{
        unsigned int val = 0;
        if (led_status == LED_ON) {
                val = readl(GPIO1_DR);
                val &= ~(1<<3);
                writel(val, GPIO1_DR);
        } else {
                val = readl(GPIO1_DR);
                val |= (1<<3);
                writel(val, GPIO1_DR);
        }
}

static void led_gpio_init(void)
{
        unsigned int val = 0;

        /* 完成内存地址映射 */
        CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
        SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
        SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
        GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
        GPIO1_DIR = ioremap(GPIO1_DIR_BASE, 4);

        /* 完成初始化 */
        /* 配置时钟*/
        val = readl(CCM_CCGR1);
        val |= (3<<26);
        writel(val, CCM_CCGR1);

        /* 初始化IO复用 */
        writel(0x05, SW_MUX_GPIO1_IO03);

        /* 配置IO属性 */
        writel(0x10B0, SW_PAD_GPIO1_IO03);
        
        /* 配置为输出模式 */
        val = readl(GPIO1_DIR);
        val |= (1<<3);
        writel(val, GPIO1_DIR);

        led_switch(LED_OFF);
}

static int led_open(struct inode *inode, struct file *file)
{
        printk("led open!\n");
        return 0;
}

static ssize_t led_read(struct file *file, char __user *user, size_t size, loff_t *loff)
{
        return 0;
}

static ssize_t led_write(struct file *file, const char __user *user, size_t size, loff_t *loff)
{
        int ret = 0;
        unsigned char buf[1];
        ret = copy_from_user(buf, user, 1);
        if (ret < 0) {
                printk("kernel write failed!\n");
                ret = -EFAULT;
                goto error;
        }

        if((buf[0] != LED_OFF) && (buf[0] != LED_ON)) {
                ret = -1;
                goto error;
        }
        led_switch(buf[0]);

error:
        return ret;
}

static int led_release(struct inode *inode, struct file *file)
{
        printk("led release!\n");
        return 0;
}

static int __init led_init(void)
{
        int ret = 0;

        printk("led module init!\n");

        led_gpio_init();
        ret = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
        if (ret == -1) {
                printk("register_chrdev error");
                goto error;
        }

error:
        return ret;
}

static void __exit led_exit(void)
{
        printk("led module exit!\n");

        /* 关闭LED灯 */
        led_switch(LED_OFF);

        iounmap(CCM_CCGR1);
        iounmap(SW_MUX_GPIO1_IO03);
        iounmap(SW_PAD_GPIO1_IO03);
        iounmap(GPIO1_DR);
        iounmap(GPIO1_DIR);

        unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanglei");
