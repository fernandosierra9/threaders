################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include bin/subdir.mk
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
all: libsac_cli.so

dependents:
	-cd /media/sf_tp-2019-2c-threaders/shared-common/Debug && $(MAKE) all

# Tool invocations
libsac_cli.so: $(OBJS) $(USER_OBJS) /media/sf_tp-2019-2c-threaders/shared-common/Debug/libshared-common.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/media/sf_tp-2019-2c-threaders/shared-common/Debug" -shared -o "libsac_cli.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) libsac_cli.so
	-@echo ' '

.PHONY: all clean dependents
/media/sf_tp-2019-2c-threaders/shared-common/Debug/libshared-common.so:

-include ../makefile.targets
