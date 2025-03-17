// Copyright (c) 2025 Vladimir Fedorov

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// This header file contains the implementation of all memctx functions.

#ifndef _MEMCTX_H_
#define _MEMCTX_H_

#include <cstddef>
#include <cstdint>
#include <stddef.h>

#define MEMCTX_PAGE_SIZE 4069			// One memory page
										// When the requested allocation size is smaller than
										// MEMCTX_PAGE_SIZE, memctx allocates memory pages incrementally:
										// - The first allocation is always MEMCTX_MIN_CAPACITY.
										// - The second allocation is twice the previous size
										//   (MEMCTX_MIN_CAPACITY * 2).
										// - Each subsequent allocation doubles again 
										// (MEMCTX_MIN_CAPACITY * 4), and so forth,
										// until MEMCTX_MAX_CAPACITY is reached.
										//
#define MEMCTX_MIN_CAPACITY 4096		// Minimum allocatable capacity
#define MEMCTX_MAX_CAPACITY 4194304		// Maximum allocatable capacity
										//
										// When the requested allocation size exceeds (capacity - consumed)
										// and is greater than twice the current capacity (capacity * 2),
										// memctx allocates memory by rounding up the requested size
										// to the nearest multiple of MEMCTX_PAGE_SIZE.

typedef struct {
	uintptr_t *data;
	size_t capacity;
	size_t consumed;
	uintptr_t *next;
} MemContext;

// - Main -

void* memctx();
void* memctx_alloc(MemContext *memctx, size_t size);
void  memctx_free(MemContext *memctx);
char* memctx_description(MemContext *memctx);

// - Helper Functions -

void* memctx_sprintf(MemContext *memctx, const char *format, ...);
void* memctx_open_file(MemContext *memctx, char *filename);
void  memctx_free_file(uintptr_t *memctx_file);

#endif // _MEMCTX_H_