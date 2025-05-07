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

// This header file contains the implementation of all array functions.

#ifndef _MEMCTX_ARRAYS_H_
#define _MEMCTX_ARRAYS_H_

#include <stddef.h>
#include <ctype.h>
#include "memctx.h"

#ifndef ARRAY_INIT_CAPACITY
#define ARRAY_INIT_CAPACITY 4
#endif

typedef struct memctx_array {
    void **items;
    size_t length;
    size_t capacity;
    MemContext *ctx;
} array;

typedef bool (*Comparator)(void *item);
typedef void (*Action)(void *item);

array* array_init(MemContext *ctx);
void array_free(array *arr);
void array_clear(array *arr);

size_t array_append(array *arr, void *item);
void array_insert_at(array *arr, void *item, size_t index);
void array_remove_at(array *arr, size_t index);
void* array_item_at(array *arr, size_t index);

size_t array_first_index(array *arr, Comparator cmp);
void array_match(array *arr, Comparator cmp, Action action);
void array_foreach(array *arr, Action action);
void array_remove(array *arr, Comparator cmp);


void __array_resize(array *arr, size_t capacity); // internal

#endif