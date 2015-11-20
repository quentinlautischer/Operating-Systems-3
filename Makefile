CC = gcc-4.8
CFLAGS = -Wall
MEMFLAGS = -DMEMWATCH -DMW_STDIO

INCLUDES = -I memwatch-2.71/ memwatch-2.71/memwatch.c  -I src/libs/ src/libs/linkedList.c -I src/libs/ src/libs/monLL.c

TARGET = procnanny

SRC = src/$(TARGET).c		\
		src/childNanny.c 	\
		src/clerkNanny.c 	\
		src/clientNanny.c 	\

all: $(TARGET)

$(TARGET): 
	$(CC) $(CFLAGS) $(MEMFLAGS) $(INCLUDES) -o bin/$(TARGET) $(SRC) 


clean:
	$(RM) bin/$(TARGET)

run:
	bin/procnanny src/tmp/a2-simple-test.config
