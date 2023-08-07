#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"

typedef int pid_t;

static void syscall_handler (struct intr_frame *);

/* system call functions */

/*
void halt(void);
void exit(int);
pid_t exec(const char*);
int wait(pid_t);
bool create(const char*, unsigned int);
bool remove(const char*);
int open(const char*);
int filesize(int);
int write(int, const void*, unsigned int);
void seek(int, unsigned position);
unsigned int tell(int fd);
void close(int fd);
*/

void valid_check(void*);

void
syscall_init (void) 
{
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
			//hex_dump(f->esp, f->esp, 100, 1);
			valid_check(f->esp+4);
			f->eax = exec((const char*)*(uint32_t*)(f->esp+4));
			break;

		case SYS_WAIT:
			valid_check(f->esp+4);
			f->eax = wait((pid_t)*(uint32_t*)(f->esp+4));
			break;
		
		/*
		case SYS_CREATE:
			break;

		case SYS_REMOVE:
			break;

		case SYS_OPEN:
			break;

		case SYS_FILESIZE:
			break;
		*/

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

		/*
		case SYS_SEEK:
			break;
		case SYS_TELL:
			break;
		case SYS_CLOSE:
			break;

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
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
	thread_exit();
}

pid_t exec(const char* cmd_line) {
	return process_execute(cmd_line);
}

int wait(pid_t pid) {
	return process_wait(pid);
}

/*

bool create(const char* file, unsigned initial_size) {
	return false;
}

bool remove(const char* file) {
	return 0;
}

int open(const char* file) {
	return 0;	
}

int filesize(int fd) {
	return 0;
}

*/

int read(int fd, void* buffer, unsigned int size) {
	unsigned int i;
	if(fd == 0) {
		for(i = 0; i < size; i++) {
			((char*)buffer)[i] = (char)input_getc();
			if(((char*)buffer)[i] == '\0') break;
		}
		return i;
	}
	return -1;
}

int write(int fd, const void* buffer, unsigned int size) {
	if(fd == 1) {
		putbuf(buffer, size);
		return size;
	}
	return -1;
}

/*

void seek(int fd, unsigned position) {
}

unsigned int tell(int fd) {
	return 0;
}

void close(int fd) {
}

*/