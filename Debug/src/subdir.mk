################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/testlockfreequeue.cpp \
../src/udpserver.cpp 

OBJS += \
./src/testlockfreequeue.o \
./src/udpserver.o 

CPP_DEPS += \
./src/testlockfreequeue.d \
./src/udpserver.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__DEBUG__ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -march=native -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


