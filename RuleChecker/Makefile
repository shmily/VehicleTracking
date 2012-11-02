INCLUDE_FLAGS = -I./include -I../GPS/nmea -I../include -I../ImageTransmit/include -I../task -I../ReportTransmit/include -I../protocol/include -I../ReportTransmit/include -I../alarm
CFLAGS	      = $(INCLUDE_FLAGS) -Wall -g
CC			  = arm-linux-gcc

AR			  = arm-linux-ar
AFLAGS		  = -r

TARGET = libRuleChecker.a

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
