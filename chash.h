#ifdef CHASH_H
#define CHASH_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef CHASH_MALLOC
#define CHASH_MALLOC(size) malloc(size)
#endif

#ifndef CHASH_FREE
#define CHASH_FREE(ptr) free(ptr)
#endif

typedef struct chash_bucket {
	struct chash_bucket *next;
	void *data;
} chash_bucket_t;

typedef struct chash {
	chash_bucket_t **index;
	uint32_t size;
	uint32_t count;
	uint32_t (*key)(void *);
#ifdef CHASH_THREAD_SAFE
	volatile uint32_t *locks;
	volatile uint32_t lock;
#endif
} chash_t;

#define CHASH_BUCKET_LOOKUP(hash_table, hash_code, out_bucket, out_front_bucket compare, ...) ({ \
	__label__ OUT; \
	out_bucket = hash_table->index[hash_code%hash_table->size]; \
	out_front_bucket = out_bucket;\
	while(out_bucket) { \
		if(!compare(out_bucket->data, __VA_ARGS__)) \
			goto OUT; \
		out_front_bucket = out_bucket;\
		out_bucket = out_bucket->next; \
	} \
	(void *)0 \
	OUT: \
	out_bucket->data; \
})

#define CHASH_BUCKET_CALLBACK_LOOKUP(hash_table, hash_code, compare, callback, arg_in, arg_out, ...) \
do { \
	chash_bucket_t *bucket = hash_table->index[hash_code%hash_table->size]; \
	while(bucket) { \
		if(!compare(bucket->data, __VA_ARGS__)) \
			if(callback(bucket->data, arg_in, arg_out) \
				break; \
		bucket = bucket->next; \
	} \
} while(0)

#define CHASH_LOOKUP(hash_table, hash_code, compare, ...) ({ \
	chash_bucket_t *out_bucket = NULL; \
	CHASH_BUCKET_LOOKUP(hash_table, hash_code, out_bucket, out_front_bucket compare,__VA_ARGS__) \
})

#define CHASH_CALLBACK_LOOKUP(hash_table, compare, callback, arg_in, arg_out, ...) \
do { \
	uint32_t index; \
	for(index = 0; index < hash_table->size; ++index) \
		CHASH_BUCKET_CALLBACK_LOOKUP(hash_table, index, compare, callback, arg_in, arg_out, __VA_ARGS__) \
} while(0)

#define CHASH_INSERT(hash_table, hash_code)

#define CHASH_DELETE(hash_table, hash_code, compare, free_func ...) \
do { \
	__label__ OUT; \
	chash_bucket_t *out_bucket = NULL; \
	chash_bucket_t *out_front_bucket = NULL; \
	CHASH_BUCKET_LOOKUP(hash_table, hash_code, out_bucket, out_front_bucket compare,__VA_ARGS__)); \
	if (!out_bucket) { \
		break; \
	} \
	if (out_bucket == out_front_bucket) { \
		out_front_bucket = NULL; \
	} else { \
		out_front_bucket->next = out_bucket->next; \
	} \
	free_func(out_bucket->data); \
	CHASH_FREE(out_bucket); \
} while(0)

#endif