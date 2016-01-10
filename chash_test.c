#include <assert.h>
#include <stdio.h>
#include "chash.h"

typedef struct test_elem {
	int32_t value;
} test_elem_t;

typedef struct test_arg {
	int32_t sum;
	int32_t cnt;
} test_arg_t;

chash_t *test_ht;

#define HTTEST_LOCK() \
	CHASH_LOCK(test_ht)
#define HTTEST_UNLOCK() \
	CHASH_UNLOCK(test_ht)

#define HTTEST_BUCKET_LOCK(hash_code) \
	CHASH_BUCKET_LOCK(test_ht, hash_code)
#define HTTEST_BUCKET_UNLOCK(hash_code) \
	CHASH_BUCKET_UNLOCK(test_ht, hash_code)

#define HTTEST_LOOKUP_WITHOUT_LOCK(hash_code, value) \
	CHASH_LOOKUP(test_ht, hash_code, test_compare, value)
	
#define HTTEST_BUCKET_CALLBACK_WITHOUT_LOCK(hash_code, compare, callback, arg_in, arg_out, value) \
	CHASH_BUCKET_CALLBACK(test_ht, hash_code, compare, callback, arg_in, arg_out, value)

#define HTTEST_CALLBACK_WITHOUT_LOCK(compare, callback, arg_in, arg_out, value) \
	CHASH_CALLBACK(test_ht, compare, callback, arg_in, arg_out, value)

#define HTTEST_INSERT_WITHOUT_LOCK(hash_code, value) \
	CHASH_INSERT(test_ht, hash_code, test_init, value)

#define HTTEST_DELETE_WITHOUT_LOCK(hash_code, value) \
	CHASH_DELETE(test_ht, hash_code, test_compare, test_destroy, value)

static inline uint32_t test_hashcode(int32_t value)
{
	return value;
}

static inline int32_t test_compare(const test_elem_t *elem, int32_t value)
{
	assert(elem);
	return elem->value - value;
}

static inline int32_t test_compare_null(const test_elem_t *elem, int32_t value)
{
	assert(elem);
	return 0;
}

static inline int32_t test_init(test_elem_t *elem, int32_t value)
{
	assert(elem);
	elem->value = value;
	return 0;
}

static inline void test_destroy(test_elem_t *elem)
{
	assert(elem);
}

static inline int32_t test_callback(test_elem_t *elem, void *in, void *out)
{
	assert(elem);
	printf("%d ", elem->value);
	test_arg_t *arg = out;
	if (arg) {
		arg->sum += elem->value;
		++arg->cnt;
	}
	return 0;
}

int main()
{
	srand(100);
	test_ht = chash_create(sizeof(test_elem_t));
	if (test_ht == NULL)
		printf("create chash failed.\n");

	int32_t i, ret = 0, values[10];
	uint32_t hash_code;
	test_arg_t arg;
	test_elem_t *elem = NULL;

	for (i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
		values[i] = rand() & 0x0000FFFF;
	}

printf("insert test:\n\tinsert: ");
	for (i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
		hash_code = test_hashcode(values[i]);
		ret = HTTEST_INSERT_WITHOUT_LOCK(hash_code, values[i]);
		assert(!ret);
		printf("%d ", values[i]);
	}

printf("\n\nlookup test:\n\tlookup: ");
	for (i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
		hash_code = test_hashcode(values[i]);
		elem = HTTEST_LOOKUP_WITHOUT_LOCK(hash_code, values[i]);
		if (elem)
			printf("%d ", elem->value);
	}

printf("\n\ncallback test:\n\tcallback: ");
	memset(&arg, 0, sizeof(arg));
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, &arg, 0);
	printf("\n\tsum %d, cnt %d\n", arg.sum, arg.cnt);

	printf("\ndelete test:\n\tdelete: ");
	for (i = 0; i < sizeof(values)/sizeof(values[0]) - 3; ++i) {
		hash_code = test_hashcode(values[i]);
		ret = HTTEST_DELETE_WITHOUT_LOCK(hash_code, values[i]);
		assert(!ret);
		printf("%d ", values[i]);
	}
	printf("\n\tleave: ");
	memset(&arg, 0, sizeof(arg));
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, &arg, 0);
	printf("\n\tsum %d, cnt %d\n", arg.sum, arg.cnt);

printf("\nclean test:\n");
	chash_clean(test_ht);
	printf("\tleave: ");
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, NULL, 0);
	printf("\n", arg.sum, arg.cnt);
	return 0;
}
