################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/file_system/bitmap.c \
../src/file_system/bloques_handler.c \
../src/file_system/game_card_file_system.c \
../src/file_system/game_card_handler.c \
../src/file_system/setup.c 

OBJS += \
./src/file_system/bitmap.o \
./src/file_system/bloques_handler.o \
./src/file_system/game_card_file_system.o \
./src/file_system/game_card_handler.o \
./src/file_system/setup.o 

C_DEPS += \
./src/file_system/bitmap.d \
./src/file_system/bloques_handler.d \
./src/file_system/game_card_file_system.d \
./src/file_system/game_card_handler.d \
./src/file_system/setup.d 


# Each subdirectory must supply rules for building sources it contributes
src/file_system/%.o: ../src/file_system/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2020-1c-CDev20/shared-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


