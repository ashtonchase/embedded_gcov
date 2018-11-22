/*
 * @file embedded_gcov.h
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef EMBEDDED_GCOV_H_
#define EMBEDDED_GCOV_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "ff.h"
#include "xil_printf.h"

#define ENABLE_COVERAGE   1  /* 1 to enable coverage, 0 to disable it */


#define MAX_NUM_OF_GCOV_OPEN_FILES 10
#define MAX_FILENAME_SIZE 64

void static_init(void);

void _exit(int);

#endif /* EMBEDDED_GCOV_H_ */
