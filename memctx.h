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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef MEMCTX_PAGE_SIZE
#define MEMCTX_PAGE_SIZE 4069
#endif

#define MEMCTX_DESC_FORMAT "%p: capacity: %zu consumed: %zu data: %p next: %p\n"

typedef struct {
    char *data;
    size_t capacity;
    size_t consumed;
    uintptr_t *next;
} MemContext;

// - Main -

/**
 * Create a new memory context.
 *
 * @return Pointer to the newly created MemContext, or NULL if allocation fails
 */
void* memctx();

/**
 * Allocate memory within a memory context.
 *
 * @param memctx Pointer to the memory context
 * @param size Number of bytes to allocate
 * @return Pointer to the allocated memory, or NULL if:
 *         - memctx is NULL
 *         - size is 0
 *         - allocation fails
 */
void* memctx_alloc(MemContext *memctx, size_t size);

/**
 * Free all memory associated with a memory context.
 * Frees all blocks in the context and the context itself.
 *
 * @param memctx Pointer to the memory context to free
 */
void  memctx_free(MemContext *memctx);

/**
 * Generate a string description of a memory context.
 * Includes details about each block in the context.
 *
 * @param memctx Pointer to the memory context
 * @return Heap-allocated string containing the description,
 *         which must be freed by the caller, or NULL if memctx is NULL
 */
char* memctx_description(MemContext *memctx);

// - Helper Functions -

/**
 * Format a string using snprintf and store it in a memory context.
 *
 * @param memctx Pointer to the memory context
 * @param buffer Pointer to a char* that will receive the formatted string
 * @param format Printf-style format string
 * @param ... Arguments to format
 * @return Size of the formatted string including null terminator, or 0 if:
 *         - memctx is NULL
 *         - format is NULL
 *         - allocation fails
 */
size_t memctx_snprintf(MemContext *memctx, char **buffer, const char *format, ...);

/**
 * Read a file into memory and store it in a memory context.
 * Creates a new block in the context to hold the file contents.
 *
 * @param memctx Pointer to the memory context
 * @param buffer Pointer to a char* that will receive the file contents
 * @param filename Name of the file to read
 * @return Size of the file in bytes, or 0 if:
 *         - memctx is NULL
 *         - filename is NULL
 *         - file cannot be opened or read
 */
size_t memctx_open_file(MemContext *memctx, char **buffer, char *filename);

/**
 * Free a file block from a memory context.
 * Removes and frees the block containing the specified file data.
 *
 * @param ctx Pointer to the memory context
 * @param memctx_file Pointer to the file data to free
 */
void   memctx_free_file(MemContext *ctx, char *memctx_file);

// - Diagnostics

/**
 * Count the number of memory blocks in a memory context.
 *
 * @param memctx Pointer to the memory context to count blocks in
 * @return The number of blocks in the memory context, or 0 if memctx is NULL
 */
int __memctx_blocks_count(MemContext *memctx);

/**
 * Get a memory block at the specified index from a memory context.
 * Supports both positive and negative indices:
 * - Positive indices (0, 1, 2, ...) count from the beginning
 * - Negative indices (-1, -2, ...) count from the end (-1 is the last block)
 *
 * @param memctx Pointer to the memory context
 * @param index Index of the block to retrieve (can be negative to count from end)
 * @return Pointer to the memory block at the specified index, or NULL if:
 *         - memctx is NULL
 *         - index is out of bounds
 *         - memory context has no blocks
 */
MemContext* __memctx_block_at(MemContext *memctx, int index);

// - Implementation -

void* memctx() {
    MemContext* ctx = (MemContext*)malloc(sizeof(MemContext));
    if (!ctx) return NULL;

    // Initialize the first memory block
    ctx->capacity = MEMCTX_PAGE_SIZE;
    ctx->consumed = 0;
    ctx->data = (char *)malloc(ctx->capacity);

    if (!ctx->data) {
        free(ctx);
        return NULL;
    }

    ctx->next = NULL;
    return ctx;
}

void* memctx_alloc(MemContext *ctx, size_t size) {
    if (!ctx || size == 0) return NULL;

    // Align size to ensure proper alignment
    size_t aligned_size = (size + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);

    // Search for a block with enough space
    MemContext *current = ctx;
    MemContext *prev = NULL;

    while (current) {
        // Check if current block has enough space
        if (current->capacity - current->consumed >= aligned_size) {
            // Found a block with enough space
            void* ptr = (void*)((char*)current->data + current->consumed);
            current->consumed += aligned_size;
            return ptr;
        }

        // Move to the next block
        prev = current;
        current = (MemContext*)current->next;
    }

    // Reached the end of the linked list, need to allocate a new block
    size_t new_capacity = MEMCTX_PAGE_SIZE;
    if (aligned_size > new_capacity) {
        new_capacity = ((aligned_size + MEMCTX_PAGE_SIZE - 1) / MEMCTX_PAGE_SIZE) * MEMCTX_PAGE_SIZE;
    }

    // Create a new block
    MemContext* new_block = (MemContext*)malloc(sizeof(MemContext));
    if (!new_block) return NULL;

    new_block->capacity = new_capacity;
    new_block->consumed = aligned_size;

    new_block->data = (char*)malloc(new_block->capacity);
    if (!new_block->data) {
        free(new_block);
        return NULL;
    }

    new_block->next = NULL;

    // Link the new block to the end of the list
    prev->next = (uintptr_t*)new_block;

    // Return a pointer to the allocated memory in the new block
    return (void*)new_block->data;
}

void memctx_free(MemContext *ctx) {
    if (!ctx) return;

    // Traverse the linked list and free each block
    MemContext *current = ctx;
    MemContext *next;

    while (current) {
        next = (MemContext*)current->next;

        if (current->data) {
            free(current->data);
        }

        free(current);
        current = next;
    }
}

char* memctx_description(MemContext *ctx) {
    if (!ctx) return NULL;

    // First pass: buffer size
    size_t buffer_size = 0;
    size_t buffer_offset = 0;
    MemContext *current = ctx;
    while (current) {
        buffer_size += snprintf(NULL, 0, MEMCTX_DESC_FORMAT,
            current, current->capacity, current->consumed, current->data, current->next);
        current = (MemContext*)current->next;
    }

    // Allocate buffer
    char* description = (char*)malloc(buffer_size + 1);
    if (!description) return NULL;
    description[buffer_size] = 0;

    // Second pass: fill the buffer
    current = ctx;
    size_t string_len;
    while (current) {
        buffer_offset += snprintf(description + buffer_offset, buffer_size, MEMCTX_DESC_FORMAT,
            current, current->capacity, current->consumed, current->data, current->next);
        current = (MemContext*)current->next;
    }

    return description;
}

size_t memctx_snprintf(MemContext *ctx, char **buffer, const char *format, ...) {
    if (!ctx || !format) {
        *buffer = NULL;
        return 0;
    }

    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);

    size_t size = vsnprintf(NULL, 0, format, args1) + 1; // +1 for null terminator
    va_end(args1);

    *buffer = (char *)memctx_alloc(ctx, size);
    if (!*buffer) {
        va_end(args2);
        return 0;
    }

    vsnprintf(*buffer, size, format, args2);
    va_end(args2);

    return size;
}

size_t memctx_open_file(MemContext *ctx, char **buffer, char *filename) {
    if (!ctx || !filename) {
        *buffer = NULL;
        return  0;;
    }

    // Open the file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        *buffer = NULL;
        return 0;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        *buffer = NULL;
        return 0;
    }

    // Create a new memory context block that's fully consumed
    MemContext* file_block = (MemContext*)malloc(sizeof(MemContext));
    if (!file_block) {
        fclose(file);
        *buffer = NULL;
        return 0;
    }

    // File block is fully consumed
    file_block->capacity = file_size;
    file_block->consumed = file_size;
    // Allocation size is a multiple of MEMCTX_PAGE_SIZE that can fit file_size + 1 for '\0'
    // Keeping +1 -1 as a reminder.
    size_t alloc_size = ((file_size + 1 + MEMCTX_PAGE_SIZE - 1) / MEMCTX_PAGE_SIZE) * MEMCTX_PAGE_SIZE;
    file_block->data = (char*)malloc(alloc_size); // <- allocate aligned size, but track only file_size
    file_block->next = NULL;

    if (!file_block->data) {
        free(file_block);
        fclose(file);
        *buffer = NULL;
        return 0;
    }

    // Read file content into the buffer
    size_t read_size = fread(file_block->data, 1, file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size) {
        free(file_block->data);
        free(file_block);
        *buffer = NULL;
        return 0;
    }

    // Make compatible with c string functions
    file_block->data[file_size] = 0;

    // Find the last block in the context
    MemContext *current = ctx;
    while (current->next) {
        current = (MemContext*)current->next;
    }

    // Link the new block to the end of the list
    current->next = (uintptr_t*)file_block;

    *buffer = (char *)file_block->data;
    return read_size;
}

void memctx_free_file(MemContext *ctx, char *memctx_file) {
    if (!ctx || !memctx_file) return;

    MemContext *prev = NULL;
    MemContext *current = ctx;

    // Find the block with data matching memctx_file
    while (current) {
        if (current->data == memctx_file) {
            break;
        }
        prev = current;
        current = (MemContext*)current->next;
    }

    if (prev) {
        prev->next = (uintptr_t *)current->next;
    }

    // Free this block
    free(current->data);
    free(current);
}

int __memctx_blocks_count(MemContext *ctx) {
    if (!ctx) return 0;

    int count = 0;
    MemContext *current = ctx;

    while (current) {
        count++;
        current = (MemContext*)current->next;
    }

    return count;
}

MemContext* __memctx_block_at(MemContext *ctx, int index) {
    if (!ctx) return NULL;

    // Get total count for negative index handling
    int count = __memctx_blocks_count(ctx);
    if (count == 0) return NULL;

    // Handle negative indices (count from end)
    if (index < 0) {
        index = count + index;
    }

    // Check if index is out of bounds
    if (index < 0 || index >= count) {
        return NULL;
    }

    // Traverse to the specified block
    MemContext *current = ctx;
    for (int i = 0; i < index; i++) {
        current = (MemContext*)current->next;
    }

    return current;
}

#endif // _MEMCTX_H_
