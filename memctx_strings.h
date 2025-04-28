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
 *  - value        A pointer to a null-terminated C string.
 * 
 * Returns a string object with the value field pointing to a newly allocated
 * memory containing a copy of the input string, and length set
 * to the length of the input string.
 */
string string_make(const char* value);

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
    string: __string_append_string, \
    char*: __string_append_chars, \
    const char*: __string_append_chars \
)(str, value)

/**
 * Reads the entire content of a file specified by the filename
 * into a string object.
 * 
 * Parameters:
 *  - filename     Path to the file to be read.
 * 
 * Returns a string object containing the contents of the file. 
 * If the file cannot be opened, or other errors occur during reading,
 * the function returns an empty `string` object.
 */
string string_read_file(const char *filename);

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

/**
 * Frees the memory allocated for the string object's value field.
 * 
 * Parameters:
 *  - str          The string object whose memory needs to be freed.
 */
void string_free(string str);

// - Implementation -
