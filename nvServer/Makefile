#############################################################################
#
#	Makefile for building the char-driver sample module
#
#############################################################################

ifeq ($(DEBUG),y)
	CFLAGS += -O -g		# -O is need to expand inlines
else
	CFLAGS += -O2
endif

TARGET	= nvServer

PWD     := $(shell pwd)
############# NV: Uncomment the following to enable cross-compile ###########################
#GUMSTIX_BUILDROOT	= $(PWD)/../gumstix-buildroot
#BUILD_ARM		= $(wildcard $(GUMSTIX_BUILDROOT)/build_arm*)
#KERNELDIR 		?= $(wildcard $(BUILD_ARM)/linux-*)
#CROSS_COMPILE		= $(patsubst %gcc, %, $(wildcard $(BUILD_ARM)/staging_dir/bin/arm-linux-uclibc*-gcc))

#TARGET_ARCH=-Os -march=armv5te -mtune=xscale -Wa,-mcpu=xscale
#CC = $(CROSS_COMPILE)gcc
################################# Cross-compile end ###################
CC = gcc
#
# If you need additional serch paths for include files, then use the -I flag
# and add them to the CPPFLAGS variable
#

# CPPFLAGS += -I somedir

#
# If you need addional search paths for library files, then use the -L flag
# and add them to LDFLAGS.
#

# LDFLAGS += -L somedir

#
# If you need additional libraries, then use -lxxx to search for libxxx.a
#

# LDLIBS += -lxxx
#LDFLAGS += -lpthread -lm	# -lpthread was giving linking error "undefined reference to `pthread_create'"
LDFLAGS += -pthread -lm		# This works

.PHONY: all

all:	depend $(TARGET)

#
# You can change the $(TARGET).c if that's not your main file and you can
# add additional .o files to the end of the line
#

SRCS	= $(TARGET).c
OBJS	= $(SRCS:.c=.o)
		     
$(TARGET) : $(OBJS)

clean:
	rm -rf $(OBJS) core .depend $(TARGET)

depend .depend dep:
	$(CC) $(CFLAGS) $(CPPFLAGS) -M $(SRCS) $(LDFLAGS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
