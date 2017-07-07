################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: LsuHeating

dependents:
	-cd /home/lsuciu/arduino/libraries/LsuLogger/Debug && $(MAKE) all
	-cd /home/lsuciu/arduino/libraries/LsuNtpTime/Debug && $(MAKE) all
	-cd /home/lsuciu/arduino/libraries/LsuScheduler/Debug && $(MAKE) all
	-cd /home/lsuciu/arduino/libraries/LsuWiFi/Debug && $(MAKE) all

# Tool invocations
LsuHeating: $(OBJS) $(USER_OBJS) /home/lsuciu/arduino/libraries/LsuLogger/Debug/LsuLogger /home/lsuciu/arduino/libraries/LsuNtpTime/Debug/LsuNtpTime /home/lsuciu/arduino/libraries/LsuScheduler/Debug/LsuScheduler /home/lsuciu/arduino/libraries/LsuWiFi/Debug/LsuWiFi
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o "LsuHeating" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(INO_DEPS)$(PDE_DEPS)$(CPP_DEPS)$(C_DEPS) LsuHeating
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/lsuciu/arduino/libraries/LsuLogger/Debug/LsuLogger:
/home/lsuciu/arduino/libraries/LsuNtpTime/Debug/LsuNtpTime:
/home/lsuciu/arduino/libraries/LsuScheduler/Debug/LsuScheduler:
/home/lsuciu/arduino/libraries/LsuWiFi/Debug/LsuWiFi:

-include ../makefile.targets
