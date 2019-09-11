################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: client-tests

dependents:
	-cd /media/sf_practica-en-c/tp-2019-2c-threaders/libmuse/Debug && $(MAKE) clean all
	-cd /home/utnso/workspace/hilolay/Debug && $(MAKE) clean all

# Tool invocations
client-tests: $(OBJS) $(USER_OBJS) /media/sf_practica-en-c/tp-2019-2c-threaders/libmuse/Debug/liblibmuse.so /home/utnso/workspace/hilolay/Debug/libhilolay.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/hilolay/Debug" -L"/media/sf_practica-en-c/tp-2019-2c-threaders/libmuse/Debug" -o "client-tests" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) client-tests
	-@echo ' '

.PHONY: all clean dependents
/media/sf_practica-en-c/tp-2019-2c-threaders/libmuse/Debug/liblibmuse.so:
/home/utnso/workspace/hilolay/Debug/libhilolay.so:

-include ../makefile.targets
