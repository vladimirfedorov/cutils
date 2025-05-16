# cutils

C Utilities

## memctx - memory context API

**memctx** implements [region-based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management),
providing a simple way to manage memory allocations.
It allows you to allocate memory within a context and then free all allocations at once, simplifying memory management.

The default size of memory blocks is 4069 bytes. You can customize the memory page size by redefining `MEMCTX_PAGE_SIZE` before including the header.

### Core Functions

#### `void* memctx()`

Creates a new memory context.

```c
// Create a new memory context
MemContext *ctx = memctx();
```

#### `void* memctx_alloc(MemContext *memctx, size_t size)`

Allocates memory within a memory context.

```c
// Allocate an integer in the memory context
int *value = memctx_alloc(ctx, sizeof(int));
*value = 42;

// Allocate a larger block of memory
void *buffer = memctx_alloc(ctx, 1024);
```

#### `void memctx_free(MemContext *memctx)`

Frees all memory associated with a memory context, including all allocations made within it.

```c
// Free the entire memory context and all its allocations
memctx_free(ctx);
```

#### `char* memctx_description(MemContext *memctx)`

Generates a string description of a memory context for debugging purposes.

```c
// Get a description of the memory context
char *desc = memctx_description(ctx);
printf("Memory context:\n%s\n", desc);
free(desc); // Must be freed by caller
```

### Helper Functions

#### `size_t memctx_snprintf(MemContext *memctx, char **buffer, const char *format, ...)`

Formats a string and stores it in the memory context.

```c
// Format a string and store it in the memory context
char *greeting;
size_t len = memctx_snprintf(ctx, &greeting, "Hello, %s!", "World");
printf("%s\n", greeting); // No need to free, will be freed with context
```

#### `size_t memctx_open_file(MemContext *memctx, char **buffer, char *filename)`

Reads a file into memory and stores it in the memory context.

```c
// Read a file into the memory context
char *file_content;
size_t file_size = memctx_open_file(ctx, &file_content, "example.txt");
if (file_size > 0) {
    printf("File content (%zu bytes):\n%s\n", file_size, file_content);
}
```

#### `void memctx_free_file(MemContext *ctx, char *memctx_file)`

Frees a specific file block from the memory context.

```c
// Free a specific file from the memory context
memctx_free_file(ctx, file_content);
```

---

## memctx_strings - string utilities

**memctx_strings** provides a string handling functions that use memory context (see `memctx.h`) for memory management.
Strings allocate memory in blocks of 256 bytes;
the size can be customized by redefining `STRING_INIT_CAPACITY` before including the header.

### String Types

- `string`: base string structure.
- `substring`: a reference to a portion of an existing string.

### String Functions

#### `string string_init(MemContext *ctx)`

Initializes a new empty string in the provided memory context.

```c
// Initializes an empty string
MemContext *ctx = memctx();
string str = string_init(ctx);
```

#### `string string_make(MemContext *ctx, const char* value)`

Creates a new string from a C string in the provided memory context.

```c
// Create a string with initial content
MemContext *ctx = memctx();
string str = string_make(ctx, "Hello, World!");
```

#### `string string_append(string str, value)`

Appends a value to a string. The value can be either a C string or a `string` struct.

```c
// Append strings together
string str = string_make("Hello");
str = string_append(str, ", ");
str = string_append(str, "World!");
```

#### `string string_read_file(MemContext *ctx, const char *filename)`

Reads an entire file into a string in the provided memory context.

```c
// Read a file into a string
MemContext *ctx = memctx();
string content = string_read_file(ctx, "example.txt");
if (content.value) {
    printf("File content: %s\n", content.value);
}
```

#### `void string_free_file(string str)`

Frees the memory allocated for a file string. This is an optimization for file strings created with `string_read_file`.
The uses `memctx_free_file` inside, freeing memory context blocks allocated for the file.

```c
// Read a file and later free its memory
MemContext *ctx = memctx();
string content = string_read_file(ctx, "example.txt");
// Use content...
string_free_file(content); // Frees the file block from the context
```

Calling this function after `string_read_file` is not necessary (memory will be freed when the memory context is deallocated anyway),
but may reduce memory footprint for long-living contexts.

#### `substring string_trim(string str)`

Trims whitespace from the beginning and end of a string, returning a substring reference.

```c
// Trim whitespace
string str = string_make("  Hello, World!  ");
substring trimmed = string_trim(str);
// trimmed now references "Hello, World!" within str
```

---

## memctx_arrays - array utilities

**memctx_arrays** provides dynamic array handling functions that use memory context (see `memctx.h`) for memory management.
Arrays allocate memory in blocks of 4 items initially;
the size can be customized by redefining `ARRAY_INIT_CAPACITY` before including the header.

### Array Types

- `array`: dynamic array structure that stores pointers to arbitrary objects.
- `Comparator`: function type for finding and filtering items in an array.
- `Action`: function type for operating on array items.

### Array Functions

#### `array* array_init(MemContext *ctx)`

Initializes a new empty array in the provided memory context.

```c
// Initializes an empty array
MemContext *ctx = memctx();
array *arr = array_init(ctx);
```

#### `void array_clear(array *arr)`

Clears all elements from an array. The array structure and its capacity are preserved.

```c
// Clear all items from an array
array_clear(arr);
// arr->length is now 0, but capacity is unchanged
```

#### `size_t array_append(array *arr, void *item)`

Appends an item to the end of the array. Automatically resizes the array if necessary.

```c
// Append items to the array
int *value = memctx_alloc(ctx, sizeof(int));
*value = 42;
size_t len = array_append(arr, value);
// len is now 1

char *str = memctx_alloc(ctx, 10);
strcpy(str, "Hello");
len = array_append(arr, str);
// len is now 2
```

#### `void array_insert_at(array *arr, void *item, size_t index)`

Inserts an item at the specified index in the array. If the index is out of bounds, the item is appended.

```c
// Insert an item at index 1
float *pi = memctx_alloc(ctx, sizeof(float));
*pi = 3.14f;
array_insert_at(arr, pi, 1);
// Order is now: [value, pi, str]
```

#### `void array_remove_at(array *arr, size_t index)`

Removes the item at the specified index from the array.

```c
// Remove the item at index 1
array_remove_at(arr, 1);
// Order is now: [value, str]
```

#### `void* array_item_at(array *arr, size_t index)`

Retrieves the item at the specified index in the array.

```c
// Get the item at index 0
int *retrieved = array_item_at(arr, 0);
printf("Value: %d\n", *retrieved); // Prints "Value: 42"
```

#### `size_t array_first_index(array *arr, Comparator cmp)`

Finds the first item in the array that satisfies the given comparator function.

```c
// Define a comparator function that matches a specific value
bool find_42(void *item) {
    return *(int*)item == 42;
}

// Find the index of the first item that equals 42
size_t index = array_first_index(arr, find_42);
// If found, index will be the position; if not found, index will be (size_t)-1
```

#### `void array_match(array *arr, Comparator cmp, Action action)`

Applies an action function to all items that match a comparator function.

```c
// Define an action function that doubles the value
void double_value(void *item) {
    *(int*)item *= 2;
}

// Double all values that equal 42
array_match(arr, find_42, double_value);
```

#### `void array_foreach(array *arr, Action action)`

Applies an action function to every item in the array.

```c
// Apply an action to all items in the array
array_foreach(arr, double_value);
// All integer values in the array are now doubled
```

#### `void array_remove(array *arr, Comparator cmp)`

Removes all items from the array that satisfy the given comparator function.

```c
// Remove all items that equal 84 (after doubling 42)
array_remove(arr, find_84);
```

Note: there's no need to call a special free function. The memory for the array and its items will be automatically freed when the memory context is freed with `memctx_free()`.
