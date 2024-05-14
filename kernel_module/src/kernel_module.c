#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/ioport.h> 
#include <linux/kobject.h>
#include <asm/errno.h> 
#include <asm/io.h> 

MODULE_INFO(intree, "Y"); 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Aleksander Pruszkowski"); 
MODULE_DESCRIPTION("Simple kernel module for SYKOM lecture"); 
MODULE_VERSION("0.01");  

#define SYKT_GPIO_BASE_ADDR  (0x00100000) 
#define SYKT_GPIO_SIZE       (0x8000)
#define SYKT_EXIT            (0x3333) 
#define SYKT_EXIT_CODE       (0x7F) 

#define SYKT_GPIO_ADDR_SPACE (SYKT_GPIO_BASE_ADDR)
#define SYKT_GPIO_A_ADDR    (SYKT_GPIO_ADDR_SPACE + 0xEC)
#define SYKT_GPIO_S_ADDR    (SYKT_GPIO_ADDR_SPACE + 0x104)
#define SYKT_GPIO_W_ADDR    (SYKT_GPIO_ADDR_SPACE + 0xFC)

// Deklaracja wskaźników na obszary pamięci 
void __iomem *baseptr;
void __iomem *baseptrA;
void __iomem *baseptrW;
void __iomem *baseptrS;

static struct kobject *kobj_ref;
static int rejAgawmat;
static int rejSgawmat;
static int rejWgawmat;

// Deklaracje funkcji
static ssize_t rejAgawmat_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t rejAgawmat_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t rejWgawmat_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t rejSgawmat_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

// ================= funkcje do komunikacji ==========================================================
// odczyt argumentu A i zapis na odpowiednie miejsce w pamieci
static ssize_t rejAgawmat_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
        sscanf(buf,"%x",&rejAgawmat);
	writel(rejAgawmat, baseptrA);
        return count;
}
// odczyt inputa z modulu
static ssize_t rejAgawmat_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%x", rejAgawmat);
}

// odczyt wyniku z modulu
static ssize_t rejWgawmat_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	rejWgawmat = readl(baseptrW);
    return sprintf(buf, "%x", rejWgawmat);
}
// odczyt statusu (czy modul skonczyl dzialanie)
static ssize_t rejSgawmat_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
	rejSgawmat = readl(baseptrS);
    return sprintf(buf, "%x", rejSgawmat);
}

// Definicje atrybutów

static struct kobj_attribute rejAgawmat_attr = __ATTR_RW(rejAgawmat);
static struct kobj_attribute rejWgawmat_attr = __ATTR_RO(rejWgawmat);
static struct kobj_attribute rejSgawmat_attr = __ATTR_RO(rejSgawmat);

int my_init_module(void){ 
    printk(KERN_INFO "Init my module.\n");
    // Inicjalizacja obszarów pamięci
    baseptr = ioremap(SYKT_GPIO_BASE_ADDR, SYKT_GPIO_SIZE);
    baseptrA = ioremap(SYKT_GPIO_A_ADDR, 2);
    baseptrW = ioremap(SYKT_GPIO_W_ADDR, 2);
    baseptrS = ioremap(SYKT_GPIO_S_ADDR, 1); 
    kobj_ref = kobject_create_and_add("sykom",kernel_kobj);
     if (!kobj_ref) {
        printk(KERN_INFO "Failed to create sysfs directory \"sykom\".\n");
	}

	if(sysfs_create_file(kobj_ref,&rejAgawmat_attr.attr)){
    	printk(KERN_INFO"Cannot create sysfs file......\n");
        kobject_put(kobj_ref);
	}

	if(sysfs_create_file(kobj_ref,&rejWgawmat_attr.attr)){
    	printk(KERN_INFO"Cannot create sysfs file......\n");
		sysfs_remove_file(kobj_ref, &rejAgawmat_attr.attr);
        kobject_put(kobj_ref);
	}

	if(sysfs_create_file(kobj_ref,&rejSgawmat_attr.attr)){
    	printk(KERN_INFO"Cannot create sysfs file......\n");
		sysfs_remove_file(kobj_ref, &rejAgawmat_attr.attr);
		sysfs_remove_file(kobj_ref, &rejWgawmat_attr.attr);
        kobject_put(kobj_ref);
	}
    return 0;
} 

void my_cleanup_module(void){ 
    printk(KERN_INFO "Cleanup my module.\n"); 
    writel(SYKT_EXIT | ((SYKT_EXIT_CODE)<<16), baseptr); 
    kobject_put(kobj_ref);
    sysfs_remove_file(kobj_ref, &rejAgawmat_attr.attr);
    sysfs_remove_file(kobj_ref, &rejWgawmat_attr.attr);
    sysfs_remove_file(kobj_ref, &rejSgawmat_attr.attr);
    iounmap(baseptr); 
    iounmap(baseptrA); 
    iounmap(baseptrW); 
    iounmap(baseptrS); 
} 

module_init(my_init_module) 
module_exit(my_cleanup_module)
