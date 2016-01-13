CC = gcc
OPT = --std=c99 -O3 -Wall -Werror
INC = 
DEFS = -DCHASH_THREAD_SAFE
CFLAGS += $(DEFS) $(INC)
LDFLAGS = 
LIBS = -lpthread

TARGET = chash_test
OBJECTS = chash_test.o

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OPT) -o $(TARGET) $(OBJECTS) $(LIBS)

.PHONY : clean
clean :
	rm -f $(TARGET) $(OBJECTS)

