DEPEND_FILE = depend_file
 
CC = gcc
 
SRCS =LED.c motor.c Server.c wavPlayer.c wavFile.h 

OBJS = $(SRCS:%.c=%.o)
 
LIBS = -lasound -lwiringPi -lpthread
 
TARGET = MyRobot
 
.SUFFIXES : .c .o
 
all : $(TARGET)
 
$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)
 
depend :
	$(CC) -MM $(SRCS) > $(DEPEND_FILE)
 
clean :
	rm -f $(OBJS) $(TARGET) $(DEPEND_FILE)
 
ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(SRCS),)
-include $(DEPEND_FILE)
endif
endif
endif
