# hash
Common hash table implementation for C

## How to use

1. include chash.h
2. define some macros and functions like in chash_test.c

## Features

### User input comparison function
The prototype of compare function can be like this: int32_t compare_func(elment_t *elment,...), and supporting more than one comparison functions.

### User input callback function
Call this function when the compare function return 0, The prototype of callback function can be like this: int32_t compare_func(elment_t *elment,...)

## Todo
### Multi-threading support
Each bucket has it's lock.

### User-defined memory management
User can set special memory management policy.