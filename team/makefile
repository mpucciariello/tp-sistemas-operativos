################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/planner/subdir.mk
-include src/logger/subdir.mk
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
all: team

dependents:
	-cd /media/sf_tp-2020-1c-CDev20/shared-common/Debug && $(MAKE) all

# Tool invocations
team: $(OBJS) $(USER_OBJS) /media/sf_tp-2020-1c-CDev20/shared-common/Debug/libshared-common.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/media/sf_tp-2020-1c-CDev20/shared-common/Debug" -o "team" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) team
	-@echo ' '

.PHONY: all clean dependents
/media/sf_tp-2020-1c-CDev20/shared-common/Debug/libshared-common.so:

-include ../makefile.targets
