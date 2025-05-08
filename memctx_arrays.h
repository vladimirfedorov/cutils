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
#include <stdbool.h>
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

/**
 * Initialize a new array with the specified memory context.
 *
 * @param ctx Pointer to the memory context to use for allocations
 * @return Pointer to the newly created array, or NULL if allocation fails
 */
array* array_init(MemContext *ctx);
void array_clear(array *arr);

size_t array_append(array *arr, void *item);
void array_insert_at(array *arr, void *item, size_t index);
void array_remove_at(array *arr, size_t index);
void* array_item_at(array *arr, size_t index);

size_t array_first_index(array *arr, Comparator cmp);
void array_match(array *arr, Comparator cmp, Action action);
void array_foreach(array *arr, Action action);
void array_remove(array *arr, Comparator cmp);

/**
 * Resizes the internal array storage to the specified capacity.
 *
 * @param arr Pointer to the array.
 * @param capacity The new capacity.
 */
void __array_resize(array *arr, size_t capacity);

// - Implementation -

array* array_init(MemContext *ctx) {
    if (!ctx) return NULL;
    
    // Allocate the array structure itself from the memory context
    array *arr = (array*)memctx_alloc(ctx, sizeof(array));
    if (!arr) return NULL;
    
    // Initialize with default values
    arr->length = 0;
    arr->capacity = ARRAY_INIT_CAPACITY;
    arr->ctx = ctx;
    
    // Allocate memory for the items array
    arr->items = (void**)memctx_alloc(ctx, sizeof(void*) * ARRAY_INIT_CAPACITY);
    if (!arr->items) {
        // No need to explicitly free arr as it's part of the memory context
        return NULL;
    }
    
    return arr;
}

/**
 * Resizes the internal array storage to the specified capacity.
 * 
 * @param arr Pointer to the array
 * @param capacity The new capacity
 */
void __array_resize(array *arr, size_t capacity) {
    if (!arr || capacity < arr->length) return;
    
    // Allocate a new items array with the increased capacity
    void **new_items = (void**)memctx_alloc(arr->ctx, sizeof(void*) * capacity);
    if (!new_items) return;
    
    // Copy existing items to the new array
    for (size_t i = 0; i < arr->length; i++) {
        new_items[i] = arr->items[i];
    }
    
    // Update the array with the new items and capacity
    // Note: We don't need to free old arr->items as it's part of the memory context
    arr->items = new_items;
    arr->capacity = capacity;
}

/**
 * Appends an item to the end of the array.
 * Automatically resizes the array if necessary.
 *
 * @param arr Pointer to the array
 * @param item The item to append
 * @return The new length of the array, or 0 if the array is NULL
 */
size_t array_append(array *arr, void *item) {
    if (!arr) return 0;
    
    // Check if we need to resize the array
    if (arr->length >= arr->capacity) {
        // Double the capacity when resizing
        __array_resize(arr, arr->capacity * 2);
        
        // Check if the resize was successful
        if (arr->length >= arr->capacity) {
            return arr->length; // Resize failed, return current length
        }
    }
    
    // Add the item to the end of the array and increment the length
    arr->items[arr->length] = item;
    arr->length++;
    
    return arr->length;
}

#endif
