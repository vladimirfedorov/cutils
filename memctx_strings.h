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

// This header file contains the implementation of all strings functions.

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#ifndef STRING_INIT_CAPACITY
#define STRING_INIT_CAPACITY 256
#endif
#include "memctx.h"

struct memctx_string {
    char *value;
    size_t length;
    size_t capacity;
    MemContext *ctx;
};
typedef struct memctx_string string;      // call string_free outside memctx
typedef struct memctx_string substring;   // do not free

/**
 * Initializes a string structure with default values.
 *
 * Returns a string object with the value set to NULL and length set to 0.
 */
string string_init(MemContext *ctx);

/**
 * Creates and returns a string object initialized
 * with the content from a null-terminated string.
 * 
 * Parameters:
 *  - ctx          The memory context to use for allocation.
 *  - value        A pointer to a null-terminated C string.
 * 
 * Returns a string object with the value field pointing to a newly allocated
 * memory containing a copy of the input string, and length set
 * to the length of the input string.
 */
string string_make(MemContext *ctx, const char* value);

/**
 * Appends a value to the end of a string.
 *
 * Parameters:
 *  - str          The destination string to append to.
 *  - value        The value to append.
 *
 * Returns the modified string with the value appended.
 */
string __string_append_string(string str, string value);

/**
 * Appends a value to the end of a string.
 *
 * Parameters:
 *  - str          The destination string to append to.
 *  - value        The value to append.
 *
 * Returns the modified string with the value appended.
 */
string __string_append_chars(string str, const char* value);

/**
 * Appends a value to the end of a string.
 * Automatically selects the appropriate function based on the type of value.
 *
 * Parameters:
 *  - str          The destination string to append to.
 *  - value        The value to append (can be either string or char*).
 *
 * Returns the modified string with the value appended.
 */
#define string_append(str, value) _Generic((value), \
    string:      __string_append_string, \
    char*:       __string_append_chars, \
    const char*: __string_append_chars \
)(str, value)

/**
 * Reads the entire content of a file specified by the filename
 * into a string object.
 * 
 * Parameters:
 *  - ctx          The memory context to use for allocation.
 *  - filename     Path to the file to be read.
 * 
 * Returns a string object containing the contents of the file. 
 * If the file cannot be opened, or other errors occur during reading,
 * the function returns an empty `string` object.
 */
string string_read_file(MemContext *ctx, const char *filename);

/**
 * Frees the memory allocated for a file string.
 * 
 * Parameters:
 *  - str          The string object whose memory needs to be freed.
 */
void string_free_file(string str);

/**
 * Trims whitespace characters from the beginning and end of a string.
 * 
 * Parameters:
 *  - str          The string to trim.
 * 
 * Returns a **substring** that references the same memory as the original string,
 * but with adjusted start position and length to exclude leading and trailing whitespace.
 */
substring string_trim(string str);

// - Implementation -

string string_init(MemContext *ctx) {
    string str;
    str.ctx = ctx;
    str.length = 0;
    str.capacity = STRING_INIT_CAPACITY;
    str.value = (char *)memctx_alloc(ctx, str.capacity);
    if (str.value) {
        str.value[0] = '\0';
    }
    return str;
}

string string_make(MemContext *ctx, const char* value) {
    string str = string_init(ctx);
    
    if (!value) return str;
    
    size_t len = strlen(value);
    if (len >= str.capacity) {
        // Need more space
        str.capacity = ((len + 1 + STRING_INIT_CAPACITY - 1) / STRING_INIT_CAPACITY) * STRING_INIT_CAPACITY;
        str.value = (char *)memctx_alloc(ctx, str.capacity);
        if (!str.value) {
            str.length = 0;
            str.capacity = 0;
            return str;
        }
    }
    
    memcpy(str.value, value, len + 1);
    str.length = len;
    return str;
}

string __string_append_string(string str, string value) {
    if (!str.value || !value.value) return str;
    
    size_t new_length = str.length + value.length;
    
    // Check if we need to expand capacity
    if (new_length >= str.capacity) {
        size_t new_capacity = ((new_length + 1 + STRING_INIT_CAPACITY - 1) / STRING_INIT_CAPACITY) * STRING_INIT_CAPACITY;
        char *new_value = (char *)memctx_alloc(str.ctx, new_capacity);
        if (!new_value) return str;  // Failed to allocate
        
        // Copy existing string and append new content
        memcpy(new_value, str.value, str.length);
        memcpy(new_value + str.length, value.value, value.length + 1);
        
        // Update the string with new buffer and capacity
        str.value = new_value;
        str.capacity = new_capacity;
    } else {
        // Enough space, just append
        memcpy(str.value + str.length, value.value, value.length + 1);
    }
    
    str.length = new_length;
    return str;
}

string __string_append_chars(string str, const char* value) {
    if (!str.value || !value) return str;
    
    size_t value_length = strlen(value);
    size_t new_length = str.length + value_length;
    
    // Check if we need to expand capacity
    if (new_length >= str.capacity) {
        size_t new_capacity = ((new_length + 1 + STRING_INIT_CAPACITY - 1) / STRING_INIT_CAPACITY) * STRING_INIT_CAPACITY;
        char *new_value = (char *)memctx_alloc(str.ctx, new_capacity);
        if (!new_value) return str;  // Failed to allocate
        
        // Copy existing string and append new content
        memcpy(new_value, str.value, str.length);
        memcpy(new_value + str.length, value, value_length + 1);  // +1 to include null terminator
        
        // Update the string with new buffer and capacity
        str.value = new_value;
        str.capacity = new_capacity;
    } else {
        // Enough space, just append
        memcpy(str.value + str.length, value, value_length + 1);  // +1 to include null terminator
    }
    
    str.length = new_length;
    return str;
}

string string_read_file(MemContext *ctx, const char *filename) {
    string str;
    str.ctx = ctx;
    str.length = 0;
    str.capacity = 0;
    str.value = NULL;
    
    if (!ctx || !filename) return str;
    
    char *file_content;
    size_t file_size = memctx_open_file(ctx, &file_content, (char *)filename);
    
    if (file_size == 0 || !file_content) {
        return str;
    }
    
    str.value = file_content;
    str.length = file_size;
    str.capacity = file_size + 1;  // +1 for null terminator which is already appended by memctx_open_file
    
    return str;
}

void string_free_file(string str) {
    if (!str.ctx || !str.value) return;
    memctx_free_file(str.ctx, str.value);
}

substring string_trim(string str) {
    substring result = {0};
    if (!str.value || str.length == 0) return result;
    
    // Point to the same string
    result.value = str.value;
    result.length = str.length;
    result.capacity = str.capacity;
    result.ctx = str.ctx;
    
    // Find the first non-whitespace character
    size_t start = 0;
    while (start < str.length && isspace((unsigned char)str.value[start])) {
        start++;
    }
    
    // If the string is all whitespace
    if (start == str.length) {
        result.value = str.value;
        result.length = 0;
        return result;
    }
    
    // Find the last non-whitespace character
    size_t end = str.length - 1;
    while (end > start && isspace((unsigned char)str.value[end])) {
        end--;
    }
    
    // Create the substring
    result.value = str.value + start;
    result.length = end - start + 1;
    
    return result;
}
