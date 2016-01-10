CC = gcc
OPT = --std=c99 -Wall -Werror
TARGET = chash_test
OBJECTS = chash_test.o

chash_test : $(OBJECTS)
	$(CC) $(OPT) -o $(TARGET) $(OBJECTS)

.PHONY : clean
clean :
	rm -f $(TARGET) $(OBJECTS)


