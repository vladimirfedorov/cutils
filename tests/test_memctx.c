#include "../memctx.h"
#include <assert.h>
#include <string.h>

void test_basic_allocation();
void test_zero_size_allocation();
void test_null_context_allocation();
void test_memctx_snprintf();
void test_memctx_snprintf_null_context();
void test_memctx_snprintf_null_format();
void test_memctx_open_file();
void test_memctx_open_nonexistent_file();
void test_memctx_free_file_null();
void test_memctx_free_null_context();
void test_memctx_blocks_count();
void test_memctx_block_at_invalid_index();
void test_large_allocation();
void test_allocation_alignment();
void test_memctx_description_null();
void test_memctx_double_free();

int main(void) {
    test_basic_allocation();
    test_zero_size_allocation();
    test_null_context_allocation();
    test_memctx_snprintf();
    test_memctx_snprintf_null_context();
    test_memctx_snprintf_null_format();
    test_memctx_open_file();
    test_memctx_open_nonexistent_file();
    test_memctx_free_file_null();
    test_memctx_free_null_context();
    test_memctx_blocks_count();
    test_memctx_block_at_invalid_index();
    test_large_allocation();
    test_allocation_alignment();
    test_memctx_description_null();
    test_memctx_double_free();

    printf("All tests completed successfully.\n");
    return 0;
}

// Test 1: Basic allocation and retrieval
void test_basic_allocation() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    int *value = memctx_alloc(ctx, sizeof(int));
    assert(value != NULL);
    *value = 42;
    assert(*value == 42);

    memctx_free(ctx);
}

// Test 2: Allocate zero bytes (should return NULL)
void test_zero_size_allocation() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    void *ptr = memctx_alloc(ctx, 0);
    assert(ptr == NULL);

    memctx_free(ctx);
}

// Test 3: Allocate with NULL context (should return NULL)
void test_null_context_allocation() {
    void *ptr = memctx_alloc(NULL, 100);
    assert(ptr == NULL);
}

// Test 4: memctx_snprintf with valid inputs
void test_memctx_snprintf() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    char *buffer;
    size_t len = memctx_snprintf(ctx, &buffer, "Test %d", 123);
    assert(len > 0);
    assert(buffer != NULL);
    assert(strcmp(buffer, "Test 123") == 0);

    memctx_free(ctx);
}

// Test 5: memctx_snprintf with NULL memctx (should return 0)
void test_memctx_snprintf_null_context() {
    char *buffer;
    size_t len = memctx_snprintf(NULL, &buffer, "Test");
    assert(len == 0);
    assert(buffer == NULL);
}

// Test 6: memctx_snprintf with NULL format (should return 0)
void test_memctx_snprintf_null_format() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    char *buffer;
    size_t len = memctx_snprintf(ctx, &buffer, NULL);
    assert(len == 0);
    assert(buffer == NULL);

    memctx_free(ctx);
}

// Test 7: memctx_open_file with valid file
void test_memctx_open_file() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    char *file_content;
    size_t len = memctx_open_file(ctx, &file_content, "test_memctx.c");
    assert(len > 0);
    assert(file_content != NULL);

    // Simple check to see if content includes "memctx"
    assert(strstr(file_content, "memctx") != NULL);

    memctx_free(ctx);
}

// Test 8: memctx_open_file with nonexistent file (should return 0)
void test_memctx_open_nonexistent_file() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    char *file_content;
    size_t len = memctx_open_file(ctx, &file_content, "nonexistent_file.txt");
    assert(len == 0);
    assert(file_content == NULL);

    memctx_free(ctx);
}

// Test 9: memctx_free_file with NULL context or file
void test_memctx_free_file_null() {
    memctx_free_file(NULL, NULL); // Should not crash
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    memctx_free_file(ctx, NULL);  // Should not crash

    char *file_content;
    size_t len = memctx_open_file(ctx, &file_content, "test_memctx.c");
    assert(len > 0);
    memctx_free_file(ctx, NULL);  // Should not crash

    memctx_free(ctx);
}

// Test 10: memctx_free with NULL context (should not crash)
void test_memctx_free_null_context() {
    memctx_free(NULL); // Should not crash
}

// Test 11: __memctx_blocks_count basic test
void test_memctx_blocks_count() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    int block_count = __memctx_blocks_count(ctx);
    assert(block_count == 1); // Should have at least one block

    // Allocate memory to potentially create new blocks
    memctx_alloc(ctx, MEMCTX_PAGE_SIZE * 2); // Allocation larger than default page size
    block_count = __memctx_blocks_count(ctx);
    assert(block_count >= 1);

    memctx_free(ctx);
}

// Test 12: __memctx_block_at with invalid index
void test_memctx_block_at_invalid_index() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    MemContext *block = __memctx_block_at(ctx, 100); // Out of bounds
    assert(block == NULL);

    block = __memctx_block_at(ctx, -100); // Out of bounds
    assert(block == NULL);

    memctx_free(ctx);
}

// Test 13: Large allocation (larger than MEMCTX_PAGE_SIZE)
void test_large_allocation() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    size_t large_size = MEMCTX_PAGE_SIZE * 10;
    void *ptr = memctx_alloc(ctx, large_size);
    assert(ptr != NULL);

    // Write to allocated memory
    memset(ptr, 0, large_size);

    memctx_free(ctx);
}

// Test 14: Allocation alignment
void test_allocation_alignment() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    // Allocate various sizes and check alignment
    size_t sizes[] = {1, 2, 4, 8, 16, 24, 32};
    for (int i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        void *ptr = memctx_alloc(ctx, sizes[i]);
        assert(ptr != NULL);
        assert(((uintptr_t)ptr % sizeof(uintptr_t)) == 0); // Check alignment
    }

    memctx_free(ctx);
}

// Test 15: memctx_description with NULL context (should return NULL)
void test_memctx_description_null() {
    char *desc = memctx_description(NULL);
    assert(desc == NULL);
}
