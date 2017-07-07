################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuWifiDeepSleep.ino 

OBJS += \
./LsuWifiDeepSleep.o 

INO_DEPS += \
./LsuWifiDeepSleep.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUBRRH -DARDUINO=100 -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/cores/esp8266 -I/home/lsuciu/arduino/libraries/LsuWiFi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


