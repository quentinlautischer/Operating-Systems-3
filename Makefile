CC = gcc-4.8
CFLAGS = -Wall
MEMFLAGS = -DMEMWATCH -DMW_STDIO

INCLUDES = -I memwatch-2.71/ memwatch-2.71/memwatch.c  -I src/libs/ src/libs/linkedList.c -I src/libs/ src/libs/monLL.c

TARGET = procnanny

TARGET_CLIENT = procnanny.client
TARGET_SERVER = procnanny.server

SRC_CLIENT = src/procnannyClient.c		\
		src/childNanny.c 	\
		src/clientNanny.c 	\

SRC_SERVER = src/procnannyServer.c		\
		src/select_servermod.c \
		src/clerkNanny.c 	\

# SRC_SERVER = src/select_servermod.c \
# 		src/clerkNanny.c 	\

all: $(TARGET)

$(TARGET): 
	$(CC) $(CFLAGS) $(MEMFLAGS) $(INCLUDES) -o bin/$(TARGET_CLIENT) $(SRC_CLIENT) 
	$(CC) $(CFLAGS) $(MEMFLAGS) $(INCLUDES) -o bin/$(TARGET_SERVER) $(SRC_SERVER)

clean:
	$(RM) bin/$(TARGET)

runS:
	#bin/procnanny.server src/tmp/a2-simple-test.config
	bin/procnanny.server 60333 src/tmp/a2-simple-test.config

runC:
	bin/procnanny.client xyz 60333