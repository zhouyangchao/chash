#chash
A general hash table implementation for C

##How to use
* include chash.h
* define some macros as interfaces and functions for comparison and callbacks like in chash_test.c

##Features
###user input comparison function
The prototype of compare function can be like this: <br>
```c
int32_t compare_func(elment_t *elment, ...)
```
and supporting more than one comparison functions while you need. It supports fuzzy comparison.
###user input callback function
Call this function when the input compare function return 0. <br>
The prototype of callback function can be like this: <br>
```c
int32_t callback_func(elment_t *elment, ...)
```
and also supporting more than one callback functions if you need.
##Todo
###User-defined memory management
User can set special memory management policy by input malloc function and free function.
