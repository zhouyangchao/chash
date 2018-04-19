/*
 * An implementation of the common hash data structure.
 *
 * Version:	@(#)chash.h	1.0.0	22/12/2015
 *
 * Authors:	Zhou Yangchao, <1028519445@qq.com>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef CHASH_H
#define CHASH_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef CHASH_THREAD_SAFE
#include <pthread.h>
#endif

#ifndef CHASH_MALLOC
#define CHASH_MALLOC(size) malloc(size)
#endif

#ifndef CHASH_FREE
#define CHASH_FREE(ptr) free(ptr)
#endif

#ifndef CHASH_BUCKET_SIZE
#define CHASH_BUCKET_SIZE 1024
#endif

typedef struct chash_bucket {
	/*pointing to next bucket*/
	struct chash_bucket *next;
	/*storing the data of element*/
	uint8_t data[];
} chash_bucket_t;

typedef struct chash {
	/*chash bucket array*/
	chash_bucket_t **index;
	/*chash bucket size, default CHASH_BUCKET_SIZE*/
	int32_t bucket_size;
	/*the count of all elememts in this chash*/
	uint32_t entity_count;
	/*the size of each element in this chash*/
	uint32_t entity_size;
#ifdef CHASH_THREAD_SAFE
	pthread_rwlock_t *locks;
	pthread_rwlock_t lock;
#endif
} chash_t;

#ifdef CHASH_THREAD_SAFE
#define CHASH_RDLOCK(hash_table) \
	pthread_rwlock_rdlock(&hash_table->lock)
#define CHASH_WRLOCK(hash_table) \
	pthread_rwlock_wrlock(&hash_table->lock)
#define CHASH_UNLOCK(hash_table) \
	pthread_rwlock_unlock(&hash_table->lock)

#define CHASH_BUCKET_RDLOCK(hash_table, hash_code) \
	pthread_rwlock_rdlock(&hash_table->locks[hash_code%hash_table->bucket_size])
#define CHASH_BUCKET_WRLOCK(hash_table, hash_code) \
	pthread_rwlock_wrlock(&hash_table->locks[hash_code%hash_table->bucket_size])
#define CHASH_BUCKET_UNLOCK(hash_table, hash_code) \
	pthread_rwlock_unlock(&hash_table->locks[hash_code%hash_table->bucket_size])
#else
#define CHASH_RDLOCK(hash_table)
#define CHASH_WRLOCK(hash_table)
#define CHASH_UNLOCK(hash_table)

#define CHASH_BUCKET_RDLOCK(hash_table, hash_code)
#define CHASH_BUCKET_WRLOCK(hash_table, hash_code)
#define CHASH_BUCKET_UNLOCK(hash_table, hash_code)
#endif

#define CHASH_BUCKET(hash_table, hash_code) \
	hash_table->index[hash_code%hash_table->bucket_size]

/*chash bucket lookup
 *@param out: elem of matched 
 *@param in : hash_table
 *@param in : hash_code
 *@param in : compare function
 *@param out: out_bucket
 *@param out: out_front_bucket
 *@param in : ...
 */
#define CHASH_BUCKET_LOOKUP(elem, hash_table, hash_code, compare, out_bucket, out_front_bucket, ...) \
do { \
__label__ FOUND;\
	out_front_bucket = CHASH_BUCKET(hash_table, hash_code); \
	out_bucket = out_front_bucket;\
	while(out_bucket) { \
		if(!compare(elem = (void *)out_bucket->data, ##__VA_ARGS__)) { \
			goto FOUND; \
		} \
		out_front_bucket = out_bucket; \
		out_bucket = out_bucket->next; \
	} \
	elem = NULL;\
FOUND:\
	(void)0; /*avoiding the label is the last statement*/\
} while(0)

/*chash bucket lookup callback
 *@param out: elem of matched 
 *@param in : hash_table
 *@param in : hash_code
 *@param in : compare function
 *@param in : callback function
 *@param in : arg_in used by callback function
 *@param in : arg_out used by callback function
 *@param in : 
 */
#define CHASH_BUCKET_CALLBACK(hash_table, hash_code, compare, callback, arg_in, arg_out, ...) ({ \
	int32_t ret = 0; \
	void *elem = NULL; \
	chash_bucket_t *bucket = CHASH_BUCKET(hash_table, hash_code); \
	while(bucket) { \
		if(!compare(elem = (void *)bucket->data, ##__VA_ARGS__) \
			&& (ret = callback(elem, arg_in, arg_out))) \
			break; \
		bucket = bucket->next; \
	} \
	ret; \
})

/*chash lookup
 *@param in : hash_table
 *@param in : hash_code
 *@param in : compare function
 *@param in : ...
 *@return elment
 */
#define CHASH_LOOKUP(hash_table, hash_code, compare, ...) ({ \
	void *elem = NULL; \
	chash_bucket_t *out_bucket = NULL; \
	CHASH_BUCKET_LOOKUP(elem, hash_table, hash_code, compare, out_bucket, out_bucket, ##__VA_ARGS__); \
	elem; \
})

/*chash lookup callback
 *@param in : hash_table
 *@param in : hash_code
 *@param in : compare function
 *@param in : callback function
 *@param in : arg_in used by callback function
 *@param in : arg_out used by callback function
 *@param in : ...
 */
#define CHASH_CALLBACK(hash_table, compare, callback, arg_in, arg_out, ...) ({ \
	int32_t ret = 0; \
	int32_t idx; \
	for(idx = 0; idx < hash_table->bucket_size; ++idx) \
		if ((ret = CHASH_BUCKET_CALLBACK(hash_table, idx, compare, callback, arg_in, arg_out, ##__VA_ARGS__))) \
			break; \
	ret; \
})

/*chash insert
 *@param in : hash_table
 *@param in : hash_code
 *@param in : init function for element
 *@param in : ...
 */
#define CHASH_INSERT(hash_table, hash_code, init, ...) ({ \
__label__ OUT; \
	int32_t ret = 1; \
	chash_bucket_t *bucket = CHASH_MALLOC(sizeof(chash_bucket_t) + hash_table->entity_size); \
	if (!bucket) { \
		goto OUT; \
	} \
	if (init((void *)bucket->data, ##__VA_ARGS__)) { \
		CHASH_FREE(bucket); \
		goto OUT; \
	} \
	bucket->next = CHASH_BUCKET(hash_table, hash_code); \
	CHASH_BUCKET(hash_table, hash_code) = bucket; \
	++hash_table->entity_count;\
	ret = 0; \
OUT: \
	ret; \
})

/*chash delete
 *@param in : hash_table
 *@param in : hash_code
 *@param in : compare function
 *@param in : destroy function for element
 *@param in : ...
 */
#define CHASH_DELETE(hash_table, hash_code, compare, destroy, ...) ({ \
	int32_t ret = 1; \
	void *elem = NULL; \
	chash_bucket_t *out_bucket = NULL; \
	chash_bucket_t *out_front_bucket = NULL; \
	CHASH_BUCKET_LOOKUP(elem, hash_table, hash_code, compare, out_bucket, out_front_bucket, ##__VA_ARGS__); \
	if (out_bucket) { \
		if (out_bucket == out_front_bucket) { \
			CHASH_BUCKET(hash_table, hash_code) = out_bucket->next; \
		} else { \
			out_front_bucket->next = out_bucket->next; \
		} \
		destroy((void *)out_bucket->data); \
		CHASH_FREE(out_bucket); \
		--hash_table->entity_count; \
		ret = 0; \
	} \
	ret; \
})

static inline chash_t *chash_create_size(uint32_t bucket_size, uint32_t entity_size)
{
	assert(bucket_size > 0);
	assert(entity_size > 0);
#ifdef CHASH_THREAD_SAFE
	int32_t i;
#endif
	chash_t *chash = (chash_t *)CHASH_MALLOC(sizeof(chash_t));
	if (NULL == chash)
		goto ERROR;
	chash->index = (chash_bucket_t **)CHASH_MALLOC(sizeof(chash_bucket_t *) * bucket_size);
	if (NULL == chash->index)
		goto ERROR_BUCKET;

	memset(chash->index, 0, sizeof(chash_bucket_t *) * bucket_size);
	chash->bucket_size = bucket_size;
	chash->entity_count = 0;
	chash->entity_size = entity_size;
#ifdef CHASH_THREAD_SAFE
	chash->locks = (pthread_rwlock_t *)CHASH_MALLOC(sizeof(pthread_rwlock_t) * bucket_size);
	if (NULL == chash->locks)
		goto ERROR_LOCK;
	if (pthread_rwlock_init(&chash->lock, NULL))
		goto ERROR_LOCK_INIT;
	for (i = 0; i < chash->bucket_size; ++i)
		if (pthread_rwlock_init(&chash->locks[i], NULL))
			goto ERROR_LOCKS_INIT;
#endif
	return chash;

#ifdef CHASH_THREAD_SAFE
ERROR_LOCKS_INIT:
	for (; i >= 0; --i)
		(void)pthread_rwlock_destroy(&chash->locks[i]);
	(void)pthread_rwlock_destroy(&chash->lock);
ERROR_LOCK_INIT:
	CHASH_FREE(chash->locks);
ERROR_LOCK:
	CHASH_FREE(chash->index);
#endif
ERROR_BUCKET:
	CHASH_FREE(chash);
ERROR:
	return NULL;
}

static inline chash_t *chash_create(uint32_t entity_size)
{
	assert(entity_size > 0);
	return chash_create_size(CHASH_BUCKET_SIZE, entity_size);
}

static inline void chash_clean(chash_t *chash)
{
	assert(chash);
	chash_bucket_t *bucket, *del_bucket;
	int32_t idx;
	for(idx = 0; idx < chash->bucket_size; ++idx) {
		bucket = chash->index[idx];
		while (bucket) {
			del_bucket = bucket;
			bucket = bucket->next;
			CHASH_FREE(del_bucket);
		}
		chash->index[idx] = NULL;
	}
}
static inline void chash_destroy(chash_t *chash)
{
	assert(chash);
#ifdef CHASH_THREAD_SAFE
	int32_t i;
#endif
	CHASH_FREE(chash->index);
#ifdef CHASH_THREAD_SAFE
	(void)pthread_rwlock_destroy(&chash->lock);
	for (i = 0; i < chash->bucket_size; ++i)
		(void)pthread_rwlock_destroy(&chash->locks[i]);
	CHASH_FREE(chash->locks);
#endif
	CHASH_FREE(chash);
}

#ifdef __cplusplus
}
#endif
#endif
