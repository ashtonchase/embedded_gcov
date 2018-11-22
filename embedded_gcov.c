/*
 * @file embedded_gcov.c
 *
 * @brief This file implements the functions needed to write the .gcda files to
 * a FAT formatted storage device.
 *
 * @see https://github.com/ashtonchase/embedded_gcov
 *
 * @par
 * MIT License
 * Copyright (c) 2018 Ashton Johnson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPresS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "embedded_gcov.h"

/* File object used by FatFS for file operations */
FIL *p_fil[MAX_NUM_OF_GCOV_OPEN_FILES];
static int fil_counter = 0;
/* File-system mounted by FatFS */
static FATFS fatfs;
/* .gcga filename generated to be written to filesystem */
char full_filename[MAX_FILENAME_SIZE];

/*
 * For FatFS logical drive 0, Path should be "0:/"
 * For FatFS logical drive 1, Path should be "1:/"
 */
TCHAR *drive_path = "0:/";

#if ENABLE_COVERAGE

/* prototype */
extern void __gcov_exit(void);

/*
 * call the coverage initializers if not done by startup code
 * @see https://mcuoneclipse.com/2014/12/26/code-coverage-for-embedded-target-with-eclipse-gcc-and-gcov/
 */
void static_init(void) {

	void (**p)(void);
	/* linker defined symbols, array of function pointers */
	extern uint32_t __init_array_start, __init_array_end;
	uint32_t beg = (uint32_t) &__init_array_start;
	uint32_t end = (uint32_t) &__init_array_end;

	while (beg < end) {
		p = (void (**)(void)) beg; /* get function pointer */
		(*p)(); /* call constructor */
		beg += sizeof(p); /* next pointer */
	}

}

void _exit(int status) {
	(void) status;
	__gcov_exit();
	for (;;) {
	} /* does not return */
}

int _write(int file, char *ptr, int len) {
	/* command line which can be used for gdb */
	static char write_buf[1024];
	uint num_of_bytes_written;

#ifdef XPARAMETERS_H
	/* helpful print function for Xilinx XSDK */
	strcpy(write_buf, full_filename);
	static char saddr[128];
	sprintf(saddr, " %p", ptr);
	strcat(write_buf, saddr);
	sprintf(saddr, " %p", (void *) len);
	strcat(write_buf, saddr);
	xil_printf("%s\r\n", write_buf);
#endif

	/*
	 * Write data to file.
	 */
	FRESULT res = f_write(p_fil[file], (const void*) ptr, len, &num_of_bytes_written);
	if (res) {
		return -1;
	} else {

		/* on success, return number of bytes written */
		return (int)num_of_bytes_written;
	}

}

int _open(const char *ptr, int mode) {

	static uint32_t fs_mounted = 0;

	if (MAX_NUM_OF_GCOV_OPEN_FILES <= fil_counter){
			return -1;
	}


	/*
	 * only mount on the first time
	 */
	if (0 == fs_mounted++) {
		/*
		 * Register volume work area, initialize device
		 */
		FRESULT res = f_mount(&fatfs, drive_path, 0);

		if (FR_OK != res) {
			fs_mounted = 0;
			return -1;
		}
	}

	/*
	 * Strip down the supplied file path give to just the file name.
	 * All .gcda files will be placed in the /coverage directory.
	 */

	const char dirname[] = "/coverage/";
	/* Find last slash in name */
	const char * stripped_name = strrchr(ptr, '/');
	/* Set basename to just after the slash */
	const char * filename = stripped_name + 1;

	strcpy(full_filename, dirname);
	/* append filename to dirname */
	strcat(full_filename, filename);


	p_fil[fil_counter] = malloc(sizeof(FIL));
	/* Open the file */
	FRESULT res = f_open(p_fil[fil_counter], full_filename,
			FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (FR_OK != res) {
		return -1;
	}

	/* Increment the number of open files */


	/*
	 * Point to beginning of file .
	 */
	res = f_lseek(p_fil[fil_counter], 0);
	if (FR_OK != res) {
		return -1;
	}
	fil_counter++;
	return fil_counter - 1;
}

int _close(int file) {

	FRESULT res = f_close(p_fil[file]);
	if (FR_OK != res) {
		return -1;
	}

	free(p_fil[file]);

	return 0;
}

int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _getpid(void) {
	return 1;
}

int _isatty(int file) {
	switch (file) {
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case STDIN_FILENO:
		return 1;
	default:
		errno = EBADF;
		return 0;
	}
}

int _kill(int pid, int sig) {

	errno = EINVAL;
	return (-1);
}

int _lseek(int file, int ptr, int dir) {
	/*
	 * Pointer to beginning of file .
	 */
	FRESULT res = f_lseek(p_fil[file], ptr);
	if (FR_OK != res) {
		return -1;
	} else {
		return ptr;
	}
}

/*
 * @see https://mcuoneclipse.com/2014/12/26/code-coverage-for-embedded-target-with-eclipse-gcc-and-gcov/
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
__attribute__((naked)) static unsigned int get_stackpointer(void) {
	__asm volatile (
			"mov r0, r13   \r\n"
			"bx lr         \r\n"
	);
}
#pragma GCC diagnostic pop

void *_sbrk(int incr) {
	extern char HeapLimit; /* Defined by the linker file */
	static char *heap_end = 0;
	char *prev_heap_end;
	char *stack;

	if (heap_end == 0) {
		heap_end = &HeapLimit;
	}
	prev_heap_end = heap_end;
	stack = (char*) get_stackpointer();

	if (heap_end + incr > stack) {
		_write(STDERR_FILENO, "Heap and stack collision\n", 25);
		errno = ENOMEM;
		return (void *) -1;
	}
	heap_end += incr;
	return (void *) prev_heap_end;
}

int _read(int file, char *ptr, int len) {
	int num_bytes_read = -1;
	FRESULT res = f_read(p_fil[file], (void*) ptr, len, &num_bytes_read);
	if (FR_OK != res) {
		return -1;
	} else {
		return num_bytes_read;
	}

}
#endif
