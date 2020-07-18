################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../common/config.c \
../common/logger.c \
../common/protocols.c \
../common/serializer.c \
../common/sockets.c \
../common/utils.c 

OBJS += \
./common/config.o \
./common/logger.o \
./common/protocols.o \
./common/serializer.o \
./common/sockets.o \
./common/utils.o 

C_DEPS += \
./common/config.d \
./common/logger.d \
./common/protocols.d \
./common/serializer.d \
./common/sockets.d \
./common/utils.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


