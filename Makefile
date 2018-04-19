CC = gcc
OPT = --std=gnu99 -O3 -Wall -Wextra -Wno-unused-parameter -Werror
INC = 
DEFS = -DCHASH_THREAD_SAFE
CFLAGS += $(DEFS) $(INC)
LDFLAGS = 
LIBS = -lpthread

TARGET = chash_test
SRCS = chash_test.c

$(TARGET) : $(SRCS)
	$(CC) $(CFLAGS) $(OPT) -o $(TARGET) $(SRCS) $(LIBS)

.PHONY : clean
clean :
	rm -f $(TARGET) chash_test.o

