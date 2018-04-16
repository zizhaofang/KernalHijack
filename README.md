# KernelHijack
to run:
1. gcc sneaky_process.c -o sneaky_process
2. make
3. sudo ./sneaky_process

In this project, I write a sneaky program and a kernal module.
The program rewrite /etc/passwd and make a original backup at /tmp/passwd, and insert the kernal module into system. After you hit keyboard with 'q', the process will quit the loop and restore the file and unload the module.
	1. The kernal module I write can hide the the executable file of sneaky process, by hijack getdents system call, and also hide from /proc (which is the directory that holds running info files for processes). You cannot find this sneaky process by "ls /proc" based on the pid, and also cannot find by "ps -a -u <user-id>", and also "ls" cannot find "sneaky_process".
	2. The kernel module will check if you are openning "/etc/passwd" and redirect that file to "/tmp/passwd". This is done by hijacking the open system call.
  	3. The kernal module will hide itself from "lsmod", by hijacking the read system call, I overwrite the line with information of "sneaky_mod"
