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
void test_array_insert_at();
void test_array_insert_at_null_array();
void test_array_insert_at_out_of_bounds();
void test_array_remove_at();
void test_array_remove_at_null_array();
void test_array_remove_at_out_of_bounds();
void test_array_clear();
void test_array_clear_null_array();
void test_array_first_index(void);
void test_array_first_index_null_array(void);
void test_array_first_index_null_comparator(void);
void test_array_first_index_no_match(void);

// Comparator functions for array_first_index tests
bool find_30(void *item);
bool find_20(void *item);
bool find_50(void *item);
bool always_true(void *item);

int main(void) {
    test_array_init();
    test_array_init_null_context();
    test_array_append();
    test_array_append_null_array();
    test_array_resize();
    test_array_item_at();
    test_array_item_at_null_array();
    test_array_item_at_out_of_bounds();
    test_array_insert_at();
    test_array_insert_at_null_array();
    test_array_insert_at_out_of_bounds();
    test_array_remove_at();
    test_array_remove_at_null_array();
    test_array_remove_at_out_of_bounds();
    test_array_clear();
    test_array_clear_null_array();
    test_array_first_index();
    test_array_first_index_null_array();
    test_array_first_index_null_comparator();
    test_array_first_index_no_match();

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

// Test 9: Test array_insert_at functionality (insert in the middle)
void test_array_insert_at() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create some test items
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    char *item2 = (char*)memctx_alloc(ctx, 10);
    strcpy(item2, "Item 2");

    char *item3 = (char*)memctx_alloc(ctx, 10);
    strcpy(item3, "Item 3");

    // Append the first and third items
    array_append(arr, item1);
    array_append(arr, item3);

    // Insert item2 at index 1 (between item1 and item3)
    array_insert_at(arr, item2, 1);

    // Verify the array length and content
    assert(arr->length == 3);
    assert(arr->items[0] == item1);
    assert(arr->items[1] == item2);
    assert(arr->items[2] == item3);

    // Verify the content of the items
    assert(strcmp((char*)arr->items[0], "Item 1") == 0);
    assert(strcmp((char*)arr->items[1], "Item 2") == 0);
    assert(strcmp((char*)arr->items[2], "Item 3") == 0);

    // Test insertion at the beginning
    char *item0 = (char*)memctx_alloc(ctx, 10);
    strcpy(item0, "Item 0");

    array_insert_at(arr, item0, 0);

    // Verify the array after insertion at the beginning
    assert(arr->length == 4);
    assert(arr->items[0] == item0);
    assert(arr->items[1] == item1);
    assert(arr->items[2] == item2);
    assert(arr->items[3] == item3);

    memctx_free(ctx);
}

// Test 10: Test array_insert_at with NULL array
void test_array_insert_at_null_array() {
    // Should not crash
    array_insert_at(NULL, (void*)"Test", 0);
}

// Test 11: Test array_insert_at with out-of-bounds index (should append)
void test_array_insert_at_out_of_bounds() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create a test item
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    // Insert at index beyond the end (should append)
    array_insert_at(arr, item1, 5);

    // Verify the array
    assert(arr->length == 1);
    assert(arr->items[0] == item1);
    assert(strcmp((char*)arr->items[0], "Item 1") == 0);

    memctx_free(ctx);
}

// Test 12: Test array_remove_at functionality
void test_array_remove_at() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create some test items
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    char *item2 = (char*)memctx_alloc(ctx, 10);
    strcpy(item2, "Item 2");

    char *item3 = (char*)memctx_alloc(ctx, 10);
    strcpy(item3, "Item 3");

    // Append all items
    array_append(arr, item1);
    array_append(arr, item2);
    array_append(arr, item3);

    // Remove the middle item
    array_remove_at(arr, 1);

    // Verify the array length and content
    assert(arr->length == 2);
    assert(arr->items[0] == item1);
    assert(arr->items[1] == item3);

    // Verify the content of the remaining items
    assert(strcmp((char*)arr->items[0], "Item 1") == 0);
    assert(strcmp((char*)arr->items[1], "Item 3") == 0);

    // Remove the first item
    array_remove_at(arr, 0);

    // Verify the array again
    assert(arr->length == 1);
    assert(arr->items[0] == item3);
    assert(strcmp((char*)arr->items[0], "Item 3") == 0);

    // Remove the last item
    array_remove_at(arr, 0);

    // Verify the array is empty
    assert(arr->length == 0);

    memctx_free(ctx);
}

// Test 13: Test array_remove_at with NULL array
void test_array_remove_at_null_array() {
    // Should not crash
    array_remove_at(NULL, 0);
}

// Test 14: Test array_remove_at with out-of-bounds index
void test_array_remove_at_out_of_bounds() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create a test item
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    // Append the item
    array_append(arr, item1);

    // Remove at index beyond the end (should do nothing)
    array_remove_at(arr, 5);

    // Verify the array is unchanged
    assert(arr->length == 1);
    assert(arr->items[0] == item1);
    assert(strcmp((char*)arr->items[0], "Item 1") == 0);

    memctx_free(ctx);
}

// Test 15: Test array_clear functionality
void test_array_clear() {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create and append some test items
    char *item1 = (char*)memctx_alloc(ctx, 10);
    strcpy(item1, "Item 1");

    char *item2 = (char*)memctx_alloc(ctx, 10);
    strcpy(item2, "Item 2");

    char *item3 = (char*)memctx_alloc(ctx, 10);
    strcpy(item3, "Item 3");

    array_append(arr, item1);
    array_append(arr, item2);
    array_append(arr, item3);

    // Verify array has items
    assert(arr->length == 3);

    // Clear the array
    array_clear(arr);

    // Verify the array is empty
    assert(arr->length == 0);

    // Verify capacity is unchanged
    assert(arr->capacity == ARRAY_INIT_CAPACITY);

    // Check that we can still append items after clearing
    array_append(arr, item1);
    assert(arr->length == 1);
    assert(arr->items[0] == item1);

    memctx_free(ctx);
}

// Test 16: Test array_clear with NULL array
void test_array_clear_null_array() {
    // Should not crash
    array_clear(NULL);
}

// Comparator function implementations
bool find_30(void *item) {
    return (*(int*)item == 30);
}

bool find_20(void *item) {
    return (*(int*)item == 20);
}

bool find_50(void *item) {
    return (*(int*)item == 50);
}

bool always_true(void *item) {
    (void)item; // Unused parameter
    return true;
}

// Test 17: Test array_first_index functionality
void test_array_first_index(void) {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create some test items
    int *item1 = (int*)memctx_alloc(ctx, sizeof(int));
    *item1 = 10;

    int *item2 = (int*)memctx_alloc(ctx, sizeof(int));
    *item2 = 20;

    int *item3 = (int*)memctx_alloc(ctx, sizeof(int));
    *item3 = 30;

    int *item4 = (int*)memctx_alloc(ctx, sizeof(int));
    *item4 = 40;

    // Append the items
    array_append(arr, item1);
    array_append(arr, item2);
    array_append(arr, item3);
    array_append(arr, item4);

    // Find the index of the item equal to 30
    size_t index = array_first_index(arr, find_30);
    assert(index == 2); // item3 is at index 2

    // Find the index of the item equal to 20
    index = array_first_index(arr, find_20);
    assert(index == 1); // item2 is at index 1

    memctx_free(ctx);
}

// Test 18: Test array_first_index with NULL array
void test_array_first_index_null_array(void) {
    size_t index = array_first_index(NULL, always_true);
    assert(index == (size_t)-1);
}

// Test 19: Test array_first_index with NULL comparator
void test_array_first_index_null_comparator(void) {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Add an item to the array
    int *item = (int*)memctx_alloc(ctx, sizeof(int));
    *item = 10;
    array_append(arr, item);

    // Call with NULL comparator
    size_t index = array_first_index(arr, NULL);
    assert(index == (size_t)-1);

    memctx_free(ctx);
}

// Test 20: Test array_first_index with no match
void test_array_first_index_no_match(void) {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    array *arr = array_init(ctx);
    assert(arr != NULL);

    // Create some test items
    int *item1 = (int*)memctx_alloc(ctx, sizeof(int));
    *item1 = 10;

    int *item2 = (int*)memctx_alloc(ctx, sizeof(int));
    *item2 = 20;

    // Append the items
    array_append(arr, item1);
    array_append(arr, item2);

    // Try to find an item that doesn't exist
    size_t index = array_first_index(arr, find_50);
    assert(index == (size_t)-1);

    memctx_free(ctx);
}
