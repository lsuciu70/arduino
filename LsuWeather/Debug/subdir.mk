################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../LsuWeather.ino 

CPP_SRCS += \
../LsuMQTT.cpp \
../MqttClient.cpp 

OBJS += \
./LsuMQTT.o \
./LsuWeather.o \
./MqttClient.o 

INO_DEPS += \
./LsuWeather.d 

CPP_DEPS += \
./LsuMQTT.d \
./MqttClient.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++98 -DARDUINO=100 -DUBRRH -I/home/lsuciu/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/cores/esp8266 -I/home/lsuciu/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/libraries/Wire -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/SparkFun_BME280/src -I/home/lsuciu/arduino/libraries/LsuWiFi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++98 -DARDUINO=100 -DUBRRH -I/home/lsuciu/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/cores/esp8266 -I/home/lsuciu/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/libraries/Wire -I/home/lsuciu/arduino/libraries/ESP8266WiFi/src -I/home/lsuciu/arduino/libraries/SparkFun_BME280/src -I/home/lsuciu/arduino/libraries/LsuWiFi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


