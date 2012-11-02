INCLUDE_FLAGS = -I./include -I../include -I../protocol/include -I../GSM -I../GPS -I../task -I../md5 -I../uart
CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g
CC			  = arm-linux-gcc

AR			  = arm-linux-ar
AFLAGS		  = -r

TARGET = libimageTransmit.a

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(AR) $(AFLAGS) $(TARGET) $(OBJS)

.PHONY:clean
clean:
	$(RM) $(TARGET) $(OBJS)
