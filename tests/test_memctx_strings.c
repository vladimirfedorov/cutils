#include "../memctx_strings.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void test_string_init(void);
void test_string_make(void);
void test_string_append(void);
void test_string_read_file(void);
void test_string_trim(void);
void test_string_free_file(void);

int main(void) {
    test_string_init();
    test_string_make();
    test_string_append();
    test_string_read_file();
    test_string_trim();
    test_string_free_file();

    printf("All string tests completed successfully.\n");
    return 0;
}

// Test 1: String initialization
void test_string_init(void) {
    MemContext *ctx = memctx();
    assert(ctx != NULL);

    string str = string_init(ctx);
    assert(str.value != NULL);
    assert(str.length == 0);
    assert(str.capacity == STRING_INIT_CAPACITY);
    assert(str.ctx == ctx);
    assert(str.value[0] == '\0');
}

// Test 2: String creation with string_make
void test_string_make(void) {
    MemContext *ctx = memctx();

    // Normal string
    string str1 = string_make(ctx, "Hello, World!");
    assert(str1.value != NULL);
    assert(str1.length == 13);
    assert(str1.capacity >= 14);  // At least length + null terminator
    assert(strcmp(str1.value, "Hello, World!") == 0);

    // Empty string
    string str2 = string_make(ctx, "");
    assert(str2.value != NULL);
    assert(str2.length == 0);
    assert(str2.capacity >= 1);  // At least space for null terminator
    assert(strcmp(str2.value, "") == 0);

    // NULL input
    string str3 = string_make(ctx, NULL);
    assert(str3.value != NULL);
    assert(str3.length == 0);

    // Long string (to test capacity handling)
    char long_string[1000];
    memset(long_string, 'A', 999);
    long_string[999] = '\0';

    string str4 = string_make(ctx, long_string);
    assert(str4.value != NULL);
    assert(str4.length == 999);
    assert(str4.capacity >= 1000);
    assert(strcmp(str4.value, long_string) == 0);

    memctx_free(ctx);
}

// Test 3: String append
void test_string_append(void) {
    // Test appending C strings
    MemContext *ctx = memctx();
    string str1 = string_init(ctx);

    str1 = string_append(str1, "Hello");
    assert(str1.length == 5);
    assert(strcmp(str1.value, "Hello") == 0);

    str1 = string_append(str1, ", ");
    assert(str1.length == 7);
    assert(strcmp(str1.value, "Hello, ") == 0);

    str1 = string_append(str1, "World!");
    assert(str1.length == 13);
    assert(strcmp(str1.value, "Hello, World!") == 0);

    // Test appending strings to each other
    MemContext *ctx2 = memctx();
    string str2 = string_init(ctx2);
    str2 = string_append(str2, "World!");

    MemContext *ctx3 = memctx();
    string str3 = string_init(ctx3);
    str3 = string_append(str3, "Hello, ");

    str3 = string_append(str3, str2);
    assert(str3.length == 13);
    assert(strcmp(str3.value, "Hello, World!") == 0);

    // Test appending to force capacity increase
    char long_string[300];
    memset(long_string, 'A', 299);
    long_string[299] = '\0';

    str1 = string_append(str1, long_string);
    assert(str1.length == 13 + 299);
    assert(str1.value[0] == 'H');
    assert(str1.value[13] == 'A');
    assert(str1.value[13 + 299 - 1] == 'A');

    // Cleanup
    memctx_free(ctx);
    memctx_free(ctx2);
    memctx_free(ctx3);
}

// Test 4: Reading a file
void test_string_read_file(void) {
    // Write a test file first
    FILE *f = fopen("test_file.txt", "w");
    if (f) {
        fputs("Test file content\nSecond line", f);
        fclose(f);
    }

    // Read the file
    MemContext *ctx = memctx();
    string str = string_read_file(ctx, "test_file.txt");
    assert(str.value != NULL);
    assert(str.length > 0);
    assert(strcmp(str.value, "Test file content\nSecond line") == 0);

    // Test with non-existent file
    string str2 = string_read_file(ctx, "nonexistent_file.txt");
    assert(str2.value == NULL);
    assert(str2.length == 0);

    // Test with NULL context
    string str3 = string_read_file(NULL, "test_file.txt");
    assert(str3.value == NULL);
    assert(str3.length == 0);

    // Test with NULL filename
    string str4 = string_read_file(ctx, NULL);
    assert(str4.value == NULL);
    assert(str4.length == 0);

    memctx_free(ctx);
    remove("test_file.txt");
}

// Test 5: String trimming
void test_string_trim(void) {
    MemContext *ctx = memctx();
    // Test with normal whitespace
    string str1 = string_make(ctx, "  Hello, World!  ");
    substring trimmed1 = string_trim(str1);
    assert(trimmed1.length == 13);
    assert(strncmp(trimmed1.value, "Hello, World!", 13) == 0);

    // Test with only whitespace
    string str2 = string_make(ctx, "   \t\n  ");
    substring trimmed2 = string_trim(str2);
    assert(trimmed2.length == 0);

    // Test with no whitespace
    string str3 = string_make(ctx, "NoWhitespace");
    substring trimmed3 = string_trim(str3);
    assert(trimmed3.length == strlen("NoWhitespace"));
    assert(strncmp(trimmed3.value, "NoWhitespace", strlen("NoWhitespace")) == 0);

    // Test with empty string
    string str4 = string_make(ctx, "");
    substring trimmed4 = string_trim(str4);
    assert(trimmed4.length == 0);

    // Test with NULL
    string str5 = {0};
    substring trimmed5 = string_trim(str5);
    assert(trimmed5.length == 0);
    assert(trimmed5.value == NULL);

    memctx_free(ctx);
}

// Test 6: File string freeing
void test_string_free_file(void) {
    // Create a test file
    FILE *f = fopen("test_file_free.txt", "w");
    if (f) {
        fputs("This is a test file for testing string_free_file", f);
        fclose(f);
    }

    // Normal case
    MemContext *ctx1 = memctx();
    string str1 = string_read_file(ctx1, "test_file_free.txt");
    string_free_file(str1);
    memctx_free(ctx1);

    // Test with non-existent file (shouldn't crash)
    MemContext *ctx2 = memctx();
    string str2 = string_read_file(ctx2, "nonexistent_file.txt");
    string_free_file(str2);
    memctx_free(ctx2);

    // Test with NULL context (shouldn't crash)
    string str3 = {0};
    string_free_file(str3);

    // Clean up test file
    remove("test_file_free.txt");
}
