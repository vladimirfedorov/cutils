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
 * Initialize a new array within the specified memory context.
 *
 * @param ctx Pointer to the memory context to use for allocations
 * @return Pointer to the newly created array, or NULL if allocation fails
 */
array* array_init(MemContext *ctx);

/**
 * Clears all elements from an array.
 * This operation resets the array length to zero but retains its capacity.
 * The array structure itself is not deallocated.
 *
 * @param arr Pointer to the array to clear
 */
void array_clear(array *arr);

/**
 * Appends an item to the end of the array.
 *
 * Automatically grows array capacity the array if necessary.
 *
 * @param arr Pointer to the array
 * @param item The item to append
 * @return The new length of the array, or 0 if the array is NULL
 */
size_t array_append(array *arr, void *item);

/**
 * Inserts an item at the specified index in the array.
 * Shifts all items at or after the index one position to the right.
 * If index is greater than or equal to the array length,
 * the item is appended to the end of the array.
 * Automatically resizes the array if necessary.
 *
 * @param arr Pointer to the array
 * @param item The item to insert
 * @param index The index at which to insert the item
 */
void array_insert_at(array *arr, void *item, size_t index);

/**
 * Removes the item at the specified index from the array.
 * Shifts all items after the index one position to the left.
 * If index is out of bounds, no action is taken.
 *
 * @param arr Pointer to the array
 * @param index The index of the item to remove
 */
void array_remove_at(array *arr, size_t index);

/**
 * Retrieves the item at the specified index in the array.
 *
 * @param arr Pointer to the array
 * @param index The index of the item to retrieve
 * @return Pointer to the item at the specified index, or NULL if:
 *         - arr is NULL
 *         - index is out of bounds
 */
void* array_item_at(array *arr, size_t index);

/**
 * Finds the first item in the array that satisfies the given comparator function.
 *
 * @param arr Pointer to the array to search
 * @param cmp The comparator function that returns true when a matching item is found
 * @return The index of the first matching item, or -1 if:
 *         - arr is NULL
 *         - cmp is NULL
 *         - no matching item is found
 */
size_t array_first_index(array *arr, Comparator cmp);

/**
 * Applies the action function to each item in the array that satisfies the given comparator function.
 *
 * @param arr Pointer to the array to search
 * @param cmp The comparator function that returns true when a matching item is found
 * @param action The action function to apply to each matching item
 * @return Nothing, but if:
 *         - arr is NULL
 *         - cmp is NULL
 *         - action is NULL (undefined behavior)
 *         then no action is taken
 */
void array_match(array *arr, Comparator cmp, Action action);

/**
 * Applies the action function to each item in the array.
 *
 * @param arr Pointer to the array to iterate over
 * @param action The action function to apply to each item
 * @return Nothing, but if:
 *         - arr is NULL
 *         - action is NULL
 *         then no action is taken
 */
void array_foreach(array *arr, Action action);

/**
 * Removes all items from the array that satisfy the given comparator function.
 *
 * @param arr Pointer to the array
 * @param cmp The comparator function that returns true for items to be removed
 * @return Nothing, but if:
 *         - arr is NULL
 *         - cmp is NULL
 *         then no action is taken
 */
void array_remove(array *arr, Comparator cmp);

/**
 * Resizes the internal array storage to the specified capacity.
 *
 * @param arr Pointer to the array
 * @param capacity The new capacity
 */
void __array_resize(array *arr, size_t capacity);

// - Implementation -

array* array_init(MemContext *ctx) {
    if (!ctx) return NULL;

    array *arr = (array*)memctx_alloc(ctx, sizeof(array));
    if (!arr) return NULL;
    arr->length = 0;
    arr->capacity = ARRAY_INIT_CAPACITY;
    arr->ctx = ctx;
    arr->items = (void**)memctx_alloc(ctx, sizeof(void*) * ARRAY_INIT_CAPACITY);
    if (!arr->items) {
        return NULL;
    }

    return arr;
}

void array_clear(array *arr) {
    if (!arr) return;
    arr->length = 0;
}

size_t array_append(array *arr, void *item) {
    if (!arr) return 0;

    // Check if we need to resize the array
    if (arr->length >= arr->capacity) {
        // Double the capacity when resizing
        __array_resize(arr, arr->capacity * 2);

        if (arr->length >= arr->capacity) {
            return arr->length; // Resize failed, return current length
        }
    }

    arr->items[arr->length] = item;
    arr->length++;

    return arr->length;
}

void array_insert_at(array *arr, void *item, size_t index) {
    if (!arr) return;

    if (index >= arr->length) {
        array_append(arr, item);
        return;
    }

    // Check if we need to resize the array
    if (arr->length >= arr->capacity) {
        __array_resize(arr, arr->capacity * 2);
        if (arr->length >= arr->capacity) {
            return;
        }
    }

    for (size_t i = arr->length; i > index; i--) {
        arr->items[i] = arr->items[i - 1];
    }
    arr->items[index] = item;
    arr->length++;
}

void array_remove_at(array *arr, size_t index) {
    if (!arr || index >= arr->length) return;
    for (size_t i = index; i < arr->length - 1; i++) {
        arr->items[i] = arr->items[i + 1];
    }
    arr->length--;
}

void* array_item_at(array *arr, size_t index) {
    if (!arr || index >= arr->length) return NULL;
    return arr->items[index];
}

size_t array_first_index(array *arr, Comparator cmp) {
    if (!arr || !cmp) return -1;

    for (size_t i = 0; i < arr->length; i++) {
        if (cmp(arr->items[i])) {
            return i;
        }
    }

    return -1;
}

void array_match(array *arr, Comparator cmp, Action action) {
    if (!arr || !cmp || !action) return;
    for (size_t i = 0; i < arr->length; i++) {
        if (cmp(arr->items[i])) {
            action(arr->items[i]);
        }
    }
}

void array_foreach(array *arr, Action action) {
    if (!arr || !action) return;
    for (size_t i = 0; i < arr->length; i++) {
        action(arr->items[i]);
    }
}

void array_remove(array *arr, Comparator cmp) {
    if (!arr || !cmp) return;

    size_t write_index = 0;
    for (size_t read_index = 0; read_index < arr->length; read_index++) {
        // If the item doesn't match the removal criteria, keep it
        if (!cmp(arr->items[read_index])) {
            // Only copy if the positions differ
            if (write_index != read_index) {
                arr->items[write_index] = arr->items[read_index];
            }
            write_index++;
        }
    }

    arr->length = write_index;
}

void __array_resize(array *arr, size_t capacity) {
    if (!arr || capacity < arr->length) return;

    void **new_items = (void**)memctx_alloc(arr->ctx, sizeof(void*) * capacity);
    if (!new_items) return;

    for (size_t i = 0; i < arr->length; i++) {
        new_items[i] = arr->items[i];
    }
    arr->items = new_items;
    arr->capacity = capacity;
}

#endif
