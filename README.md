# hash
Common hash table implementation for C

## Features

### User set comparison function
The prototype of compare function can be like this: int32_t compare_func(elment_t *elment,...)

### Multi-threading support
Each bucket has it's lock.
