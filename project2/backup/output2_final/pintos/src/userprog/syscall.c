#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"
#include "threads/synch.h"


typedef int pid_t;
typedef int32_t off_t;

struct lock file_lock;

struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

static void syscall_handler (struct intr_frame *);

/* system call functions */
void halt(void);
void exit(int);
pid_t exec(const char*);
int wait(pid_t);
bool create(const char*, unsigned int);
bool remove(const char*);
int open(const char*);
int filesize(int);
int read(int, void*, unsigned int);
int write(int, const void*, unsigned int);
void seek(int, unsigned int);
unsigned int tell(int);
void close(int);

/* additional syscall */
int fibonacci(int);
int max_of_four_int(int, int, int, int);

void valid_check(void*);

void
syscall_init (void) 
{
	lock_init(&file_lock);
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void valid_check(void* addr) {
	if(!is_user_vaddr(addr)) exit(-1);
} 

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf ("system call!\n");
	/* TODO */
	int syscall_num = (int)*(uint32_t*)(f->esp);
	switch(syscall_num) {
		// project 2
		case SYS_HALT:
			halt();
			break;

		case SYS_EXIT:
			valid_check(f->esp+4);
			exit((int)*(uint32_t*)(f->esp+4));
			break;

		case SYS_EXEC:
			valid_check(f->esp+4);
			f->eax = exec((const char*)*(uint32_t*)(f->esp+4));
			break;

		case SYS_WAIT:
			valid_check(f->esp+4);
			f->eax = wait((pid_t)*(uint32_t*)(f->esp+4));
			break;
		
		case SYS_CREATE:
			valid_check(f->esp+4);
			valid_check(f->esp+8);
			f->eax = create((const char*)*(uint32_t*)(f->esp+4), (unsigned int)*(uint32_t*)(f->esp+8));
			break;

		case SYS_REMOVE:
			valid_check(f->esp+4);
			f->eax = remove((const char*)*(uint32_t*)(f->esp+4));
			break;

		case SYS_OPEN:
			valid_check(f->esp+4);
			f->eax = open((const char*)*(uint32_t*)(f->esp+4));
			break;

		case SYS_FILESIZE:
			valid_check(f->esp+4);
			f->eax = filesize((int)*(uint32_t*)(f->esp+4));
			break;

		case SYS_READ:
			valid_check(f->esp+4);
			valid_check(f->esp+8);
			valid_check(f->esp+12);
			f->eax = read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8), (unsigned int)*(uint32_t*)(f->esp+12));
			break;

		case SYS_WRITE:
			valid_check(f->esp+4);
			valid_check(f->esp+8);
			valid_check(f->esp+12);
			f->eax = write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8), (unsigned int)*(uint32_t*)(f->esp+12));
			break;

		case SYS_FIBO:
			valid_check(f->esp+4);
			f->eax = fibonacci((int)*(uint32_t*)(f->esp+4));
			break;
		
		case SYS_MAX_FOUR:
			valid_check(f->esp+4);
			valid_check(f->esp+8);
			valid_check(f->esp+12);
			valid_check(f->esp+16);
			f->eax = max_of_four_int((int)*(uint32_t*)(f->esp+4), (int)*(uint32_t*)(f->esp+8), (int)*(uint32_t*)(f->esp+12), (int)*(uint32_t*)(f->esp+16));
			break;

		case SYS_SEEK:
			valid_check(f->esp+4);
			valid_check(f->esp+8);
			seek((int)*(uint32_t*)(f->esp+4), (unsigned int)*(uint32_t*)(f->esp+8));
			break;
		case SYS_TELL:
			valid_check(f->esp+4);
			f->eax = tell((int)*(uint32_t*)(f->esp+4));
			break;
		case SYS_CLOSE:
			valid_check(f->esp+4);
			close((int)*(uint32_t*)(f->esp+4));
			break;

		/*

		// project 3
		case SYS_MMAP:
			break;
		case SYS_MUNMAP:
			break;

		// project 4
		case SYS_CHDIR:
			break;
		case SYS_MKDIR:
			break;
		case SYS_READDIR:
			break;
		case SYS_ISDIR:
			break;
		case SYS_INUMBER:
			break;
		*/
	}
}

/* system call function */

void halt(void) {
	shutdown_power_off();
}

void exit(int status) {
	int i;
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
	for(i = 3; i < 128; i++) {
		if(thread_current()->thread_fd[i] != NULL) close(i);
	}
	thread_exit();
}

pid_t exec(const char* cmd_line) {
	return process_execute(cmd_line);
}

int wait(pid_t pid) {
	return process_wait(pid);
}

bool create(const char* file, unsigned initial_size) {
	if(file == NULL) exit(-1);
	return filesys_create(file, initial_size);
}

bool remove(const char* file) {
	return filesys_remove(file);
}

int open(const char* file) {
	if(file == NULL) exit(-1);
	valid_check(file);
	lock_acquire(&file_lock);
	int i;
	struct file* fp = filesys_open(file);
	if(fp == NULL) {
		lock_release(&file_lock);
		return -1;
	}
	else {
		if(strcmp(thread_current()->name, file) == 0) file_deny_write(fp);
		for(i = 3; i < 128; i++) {
			if(thread_current()->thread_fd[i] == NULL) {
				thread_current()->thread_fd[i] = fp;
				lock_release(&file_lock);
				return i;
			}
		}
	}
	lock_release(&file_lock);
	return -1;
}

int filesize(int fd) {
	return file_length(thread_current()->thread_fd[fd]);
}

int read(int fd, void* buffer, unsigned int size) {
	if(fd == NULL || buffer == NULL) exit(-1);
	valid_check(buffer);
	lock_acquire(&file_lock);
	int i;
	if(fd == 0) {
		for(i = 0; i < size; i++) {
			((char*)buffer)[i] = (char)input_getc();
			if(((char*)buffer)[i] == '\0') break;
		}
		lock_release(&file_lock);
		return i;
	}
	else if (fd > 2) {
		if(thread_current()->thread_fd[fd] == NULL) exit(-1);
		int result = file_read(thread_current()->thread_fd[fd], buffer, size);
		lock_release(&file_lock);
		return result;
	}
	lock_release(&file_lock);
	return -1;
}

int write(int fd, const void* buffer, unsigned int size) {
	if(fd == NULL || buffer == NULL) exit(-1);
	valid_check(buffer);
	lock_acquire(&file_lock);
	if(fd == 1) {
		putbuf(buffer, size);
		lock_release(&file_lock);
		return size;
	}
	else if (fd > 2) {
		if(thread_current()->thread_fd[fd] == NULL) exit(-1);
		int result = file_write(thread_current()->thread_fd[fd], buffer, size);
		lock_release(&file_lock);
		return result;
	}
	lock_release(&file_lock);
	return -1;
}

void seek(int fd, unsigned position) {
	file_seek(thread_current()->thread_fd[fd], position);
}

unsigned int tell(int fd) {
	return file_tell(thread_current()->thread_fd[fd]);
}

void close(int fd) {
	struct file* fp;
	if(thread_current()->thread_fd[fd] == NULL) exit(-1);
	fp = thread_current()->thread_fd[fd];
	thread_current()->thread_fd[fd] = NULL;
	file_close(fp);
}

int fibonacci(int n) {
	if(n < 0) return -1;
	else if(n == 0) return 0;
	else if(n < 3) return 1;

	int f1 = 1;
	int f2 = 1;
	int temp;
	int i;
	for(i = 0; i < n - 2; i++) {
		temp = f1 + f2;
		f1 = f2;
		f2 = temp;
	}
	return f2;
}

int max_of_four_int(int n1, int n2, int n3, int n4) {
	int max = n1;
	if(max < n2) max = n2;
	if(max < n3) max = n3;
	if(max < n4) max = n4;
	return max;
}