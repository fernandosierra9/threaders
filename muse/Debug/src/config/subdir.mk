################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config/muse_config.c 

OBJS += \
./src/config/muse_config.o 

C_DEPS += \
./src/config/muse_config.d 


# Each subdirectory must supply rules for building sources it contributes
src/config/%.o: ../src/config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2019-2c-threaders/shared-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


