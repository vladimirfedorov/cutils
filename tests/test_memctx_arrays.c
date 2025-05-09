#include "../memctx_arrays.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_array_init();
void test_array_init_null_context();
void test_array_append();
void test_array_append_null_array();
void test_array_resize();
void test_array_item_at();
void test_array_item_at_null_array();
void test_array_item_at_out_of_bounds();

int main(void) {
    test_array_init();
    test_array_init_null_context();
    test_array_append();
    test_array_append_null_array();
    test_array_resize();
    test_array_item_at();
    test_array_item_at_null_array();
    test_array_item_at_out_of_bounds();

    printf("All array tests completed successfully.\n");
    return 0;
}

// Test 1: Basic array initialization
void test_array_init() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);
    assert(arr->items != NULL);
    assert(arr->length == 0);
    assert(arr->capacity == ARRAY_INIT_CAPACITY);
    assert(arr->ctx == ctx);

    memctx_free(ctx);
}

// Test 2: Initialize with NULL context (should return NULL)
void test_array_init_null_context() {
    array *arr = array_init(NULL);
    assert(arr == NULL);
}

// Test 3: Test array_append functionality
void test_array_append() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Allocate some test items in the memory context
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    int *item2 = (int*)memctx_alloc(ctx, sizeof(int));
    *item2 = 42;

    float *item3 = (float*)memctx_alloc(ctx, sizeof(float));
    *item3 = 3.14f;

    // Append the items and verify length
    size_t len = array_append(arr, item1);
    assert(len == 1);
    assert(arr->length == 1);
    assert(arr->items[0] == item1);

    len = array_append(arr, item2);
    assert(len == 2);
    assert(arr->length == 2);
    assert(arr->items[1] == item2);

    len = array_append(arr, item3);
    assert(len == 3);
    assert(arr->length == 3);
    assert(arr->items[2] == item3);

    // Verify the items are still accessible and correct
    assert(strcmp((char*)arr->items[0], "Item 1") == 0);
    assert(*(int*)arr->items[1] == 42);
    assert(*(float*)arr->items[2] == 3.14f);

    memctx_free(ctx);
}

// Test 4: Test array_append with NULL array
void test_array_append_null_array() {
    size_t len = array_append(NULL, (void*)"Test");
    assert(len == 0);
}

// Test 5: Test array resize functionality
void test_array_resize() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Initial capacity should be ARRAY_INIT_CAPACITY
    assert(arr->capacity == ARRAY_INIT_CAPACITY);

    // Add items up to initial capacity
    for (size_t i = 0; i < ARRAY_INIT_CAPACITY; i++) {
        char *item = (char*)memctx_alloc(ctx, 20);
        sprintf(item, "Item %zu", i);
        size_t len = array_append(arr, item);
        assert(len == i + 1);
    }

    // Array should still have initial capacity
    assert(arr->capacity == ARRAY_INIT_CAPACITY);
    assert(arr->length == ARRAY_INIT_CAPACITY);

    // Adding one more item should trigger resize
    char *extra_item = (char*)memctx_alloc(ctx, 20);
    strcpy(extra_item, "Extra Item");
    size_t len = array_append(arr, extra_item);

    // Capacity should have doubled
    assert(len == ARRAY_INIT_CAPACITY + 1);
    assert(arr->capacity == ARRAY_INIT_CAPACITY * 2);
    assert(arr->length == ARRAY_INIT_CAPACITY + 1);

    // Verify all items are still accessible
    for (size_t i = 0; i < ARRAY_INIT_CAPACITY; i++) {
        char expected[20];
        sprintf(expected, "Item %zu", i);
        assert(strcmp((char*)arr->items[i], expected) == 0);
    }

    assert(strcmp((char*)arr->items[ARRAY_INIT_CAPACITY], "Extra Item") == 0);

    memctx_free(ctx);
}

// Test 6: Test array_item_at functionality
void test_array_item_at() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Allocate some test items in the memory context
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    int *item2 = (int*)memctx_alloc(ctx, sizeof(int));
    *item2 = 42;

    float *item3 = (float*)memctx_alloc(ctx, sizeof(float));
    *item3 = 3.14f;

    // Append the items
    array_append(arr, item1);
    array_append(arr, item2);
    array_append(arr, item3);

    // Test array_item_at to retrieve items
    void *retrieved1 = array_item_at(arr, 0);
    void *retrieved2 = array_item_at(arr, 1);
    void *retrieved3 = array_item_at(arr, 2);

    // Verify retrievals match original items
    assert(retrieved1 == item1);
    assert(retrieved2 == item2);
    assert(retrieved3 == item3);

    // Verify the item contents
    assert(strcmp((char*)retrieved1, "Item 1") == 0);
    assert(*(int*)retrieved2 == 42);
    assert(*(float*)retrieved3 == 3.14f);

    memctx_free(ctx);
}

// Test 7: Test array_item_at with NULL array
void test_array_item_at_null_array() {
    void *item = array_item_at(NULL, 0);
    assert(item == NULL);
}

// Test 8: Test array_item_at with out-of-bounds index
void test_array_item_at_out_of_bounds() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Append one item
    char *item = (char*)memctx_alloc(ctx, 10);
    strcpy(item, "Item 1");
    array_append(arr, item);

    // Array has length 1, so index 1 should be out of bounds
    void *retrieved = array_item_at(arr, 1);
    assert(retrieved == NULL);

    // Also test with a much larger index
    retrieved = array_item_at(arr, 100);
    assert(retrieved == NULL);

    memctx_free(ctx);
}
