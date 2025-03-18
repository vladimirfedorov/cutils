#include "../memctx.h"

int main(void) {

	MemContext *ctx = memctx();
	int *value = memctx_alloc(ctx, sizeof(int));
	*value = 4;
	printf("Location: %p, value: %d, size: %lu\n", value, *value, sizeof(typeof(*value)));

	void *a1 = memctx_alloc(ctx, 1024);
	printf("Location: %p\n", a1);

	void *a2 = memctx_alloc(ctx, 3500);
	printf("Location: %p\n", a2);

	void *a3 = memctx_alloc(ctx, 1024);
	printf("Location: %p\n", a3);

	char *string;
	size_t string_len = memctx_snprintf(ctx, &string, "Hello there: %s.", ">>>Here<<<");
	printf("%p String %lu bytes: %s\n", string, string_len, string);

	printf("Memory context:\n%s\n", memctx_description(ctx));

	char *file;
	size_t file_len = memctx_open_file(ctx, &file, "test_memctx.c");
	printf("%p File %lu bytes:\n%s\n", file, file_len, file);

	printf("Memory context:\n%s\n", memctx_description(ctx));

	void *a4 = memctx_alloc(ctx, 1024 * 10);
	printf("Location: %p\n", a4);

	void *a5 = memctx_alloc(ctx, 3000);
	printf("Location: %p\n", a5);

	printf("Memory context:\n%s\n", memctx_description(ctx));

	memctx_free_file(ctx, file);

	void *a6 = memctx_alloc(ctx, 3000);
	printf("Location: %p\n", a6);

	printf("Memory context:\n%s\n", memctx_description(ctx));

	// Test diagnostic functions
	int block_count = __memctx_blocks_count(ctx);
	printf("\nBlock count: %d\n", block_count);

	// Test getting blocks by index
	printf("\nBlocks by index:\n");
	// First block (index 0)
	MemContext *first = __memctx_block_at(ctx, 0);
	printf("First block (%p): capacity: %zu consumed: %zu\n", 
		first, first->capacity, first->consumed);
	
	// Last block (index -1)
	MemContext *last = __memctx_block_at(ctx, -1);
	printf("Last block (%p): capacity: %zu consumed: %zu\n", 
		last, last->capacity, last->consumed);

	// Middle block (if available)
	if (block_count > 2) {
		int mid_index = block_count / 2;
		MemContext *mid = __memctx_block_at(ctx, mid_index);
		printf("Middle block [%d] (%p): capacity: %zu consumed: %zu\n", 
			mid_index, mid, mid->capacity, mid->consumed);
	}

	memctx_free(ctx);

}