################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LsuLog.cpp \
../LsuWebLogger.cpp 

OBJS += \
./LsuLog.o \
./LsuWebLogger.o 

CPP_DEPS += \
./LsuLog.d \
./LsuWebLogger.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARDUINO=100 -DUBRRH -I/home/lsuciu/arduino-ide/hardware/arduino/avr/cores/arduino -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/LsuWiFi -I/home/lsuciu/arduino/libraries/LsuNtpTime -I/home/lsuciu/arduino/libraries/Time -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


