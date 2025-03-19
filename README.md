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
