################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/tests/subdir.mk
-include src/logger/subdir.mk
-include src/file_system/subdir.mk
-include src/config/subdir.mk
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
all: game-card

dependents:
	-cd /home/utnso/git/tp-2020-1c-CDev20/shared-common && $(MAKE) all

# Tool invocations
game-card: $(OBJS) $(USER_OBJS) /home/utnso/git/tp-2020-1c-CDev20/shared-common/libshared-common.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/git/tp-2020-1c-CDev20/shared-common" -o "game-card" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) game-card
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/git/tp-2020-1c-CDev20/shared-common/libshared-common.so:

-include ../makefile.targets
