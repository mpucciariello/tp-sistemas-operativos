################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/logger/game_card_logger.c 

OBJS += \
./src/logger/game_card_logger.o 

C_DEPS += \
./src/logger/game_card_logger.d 


# Each subdirectory must supply rules for building sources it contributes
src/logger/%.o: ../src/logger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2020-1c-CDev20/shared-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


