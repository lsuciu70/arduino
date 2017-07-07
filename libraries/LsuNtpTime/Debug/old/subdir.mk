################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../old/LsuNtpTime.cpp 

OBJS += \
./old/LsuNtpTime.o 

CPP_DEPS += \
./old/LsuNtpTime.d 


# Each subdirectory must supply rules for building sources it contributes
old/%.o: ../old/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/Time -I/home/lsuciu/arduino/libraries/LsuWiFi -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/Timezone -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


