#include <assert.h>
#include <stdio.h>
#include "chash.h"

#define TEST_CNT 1000

typedef struct test_elem {
	int32_t value;
} test_elem_t;

typedef struct test_arg {
	int32_t sum;
	int32_t cnt;
} test_arg_t;

chash_t *test_ht;

#define HTTEST_RDLOCK() \
	CHASH_RDLOCK(test_ht)
#define HTTEST_WRLOCK() \
	CHASH_WRLOCK(test_ht)
#define HTTEST_UNLOCK() \
	CHASH_UNLOCK(test_ht)

#define HTTEST_BUCKET_RDLOCK(hash_code) \
	CHASH_BUCKET_RDLOCK(test_ht, hash_code)
#define HTTEST_BUCKET_WRLOCK(hash_code) \
	CHASH_BUCKET_WRLOCK(test_ht, hash_code)
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

	int32_t i, ret = 0, *values = NULL;
	uint32_t hash_code;
	test_arg_t arg;
	test_elem_t *elem = NULL;

	values = (int32_t *)malloc(sizeof(int32_t) * TEST_CNT);
	assert(values);

	for (i = 0; i < TEST_CNT; ++i) {
		values[i] = rand() & 0xFFFF;
	}

printf("insert test:\n\tinsert: ");
	for (i = 0; i < TEST_CNT; ++i) {
		hash_code = test_hashcode(values[i]);
		HTTEST_BUCKET_WRLOCK(hash_code);
		ret = HTTEST_INSERT_WITHOUT_LOCK(hash_code, values[i]);
		HTTEST_BUCKET_UNLOCK(hash_code);
		assert(!ret);
		printf("%d ", values[i]);
	}

printf("\n\nlookup test:\n\tlookup: ");
	for (i = 0; i < TEST_CNT; ++i) {
		hash_code = test_hashcode(values[i]);
		HTTEST_BUCKET_RDLOCK(hash_code);
		elem = HTTEST_LOOKUP_WITHOUT_LOCK(hash_code, values[i]);
		HTTEST_BUCKET_UNLOCK(hash_code);
		if (elem)
			printf("%d ", elem->value);
	}

printf("\n\ncallback test:\n\tcallback: ");
	memset(&arg, 0, sizeof(arg));
	HTTEST_RDLOCK();
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, &arg, 0);
	HTTEST_UNLOCK();
	printf("\n\tsum %d, cnt %d\n", arg.sum, arg.cnt);

printf("\ndelete test:\n\tdelete: ");
	for (i = 0; i < TEST_CNT/2; ++i) {
		hash_code = test_hashcode(values[i]);
		HTTEST_BUCKET_WRLOCK(hash_code);
		ret = HTTEST_DELETE_WITHOUT_LOCK(hash_code, values[i]);
		HTTEST_BUCKET_UNLOCK(hash_code);
		assert(!ret);
		printf("%d ", values[i]);
	}
	printf("\n\tleave: ");
	memset(&arg, 0, sizeof(arg));
	HTTEST_RDLOCK();
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, &arg, 0);
	HTTEST_UNLOCK();
	printf("\n\tsum %d, cnt %d\n", arg.sum, arg.cnt);

printf("\nclean test:\n");
	HTTEST_WRLOCK();
	chash_clean(test_ht);
	HTTEST_UNLOCK();
	printf("\tleave: ");
	HTTEST_RDLOCK();
	HTTEST_CALLBACK_WITHOUT_LOCK(test_compare_null, test_callback, NULL, NULL, 0);
	HTTEST_UNLOCK();
	printf("\n");

	chash_destroy(test_ht);
	return 0;
}
