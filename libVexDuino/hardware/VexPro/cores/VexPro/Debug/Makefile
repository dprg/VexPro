USER_OBJS = ../../../../../libraries/Metro/Debug/Metro.o ../../../../../libraries/Messenger/Debug/Messenger.o
CPPFLAGS = -I../
CFLAGS = -I../
TERK_INCLUDE = /usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/include/terk
TERK_LIB = /usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/lib/terk

-include ../makefile.init

RM := rm -rf

# All Target
all: libvexduino.a

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Install Target
install:
	cp libvexduino.a $(TERK_LIB)
	for file in $(INSTALL_INCLUDES) ; do \
		cp $$file $(TERK_INCLUDE)  ; \
	done


# Add inputs and outputs from these tool invocations to the build variables 


# Tool invocations
libvexduino.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TerkOS Archiver (Cygwin)'
	/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/bin/ar cr libvexduino.a $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS) $(OBJS) $(USER_OBJS) $(C_DEPS) $(CC_DEPS) $(CPP_DEPS) $(EXECUTABLES) $(CXX_DEPS) \
	$(C_UPPER_DEPS) libvexduino.a
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
