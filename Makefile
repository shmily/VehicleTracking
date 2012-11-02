INCLUDE_FLAGS = -I. -I./uart -I./GSM -I./include -I./protocol/include -I./task -I./GPS/nmea -I./alarm -I./ImageTransmit/include -I./ReportTransmit/include -I./RuleChecker/include -I./DrowsyDriving
CFLAGS	     = $(INCLUDE_FLAGS) -Wall -g
CC			 = arm-linux-gcc

LFLAGS 		 = -L./RuleChecker -lRuleChecker -L./DrowsyDriving -lDrowsyDetect -L./alarm -lalarm -L./protocol -lprotocol -L./ReportTransmit -lReportTransmit -L./ImageTransmit -limageTransmit -L./GSM -lgsm -L./GPS -lgps -L./uart -luart -L./task -ltask -L./md5 -lmd5 -lm
SHELL 		 = /bin/bash
SUBDIRS 	 = uart GSM task protocol GPS alarm ImageTransmit ReportTransmit RuleChecker DrowsyDriving

TARGET = GSM-Test

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS) libs
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LFLAGS)

libs:
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE)); \
	done

.PHONY:clean update
update:
	./Update.sh $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS)
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE) clean); \
    done
