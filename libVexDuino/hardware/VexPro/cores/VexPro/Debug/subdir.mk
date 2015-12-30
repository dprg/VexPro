
# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../WString.cpp \
../main.cpp \
../vexpro_analog.cpp \
../vexpro_digital.cpp \
../qetime.cpp \
../wiring_pulse.cpp \
../vexpro_time.cpp 

SERIAL_SRCS = ../PosixSignalDispatcher.cpp \
../SerialPort.cpp \
../SerialStream.cc \
../SerialStreamBuf.cc \
../vexpro_serial.cpp

C_SRCS += \
../wiring.c \
../xtoa.c \
../wiring_shift.c

OBJS += \
./WString.o \
./main.o \
./vexpro_analog.o \
./vexpro_digital.o \
./wiring.o \
./xtoa.o  \
./wiring_pulse.o \
./wiring_shift.o \
./vexpro_time.o \
./qetime.o 


SERIAL_OBJS = ../PosixSignalDispatcher.o \
../SerialPort.o \
../SerialStream.o \
../SerialStreamBuf.o \
../vexpro_serial.o

INSTALL_INCLUDES = ../Arduino.h \
../WCharacter.h  \
../WString.h  \
../avrstdlibextras.h  \
../binary.h  \
../qetime.h  \
../wiring.h

C_DEPS += \
./wiring.d \
./xtoa.d \
./wiring_shift.d

CPP_DEPS += \
./WString.d \
./main.d \
./vexpro_analog.d \
./vexpro_digital.d \
./qetime.d \
./wiring_pulse.d \
./vexpro_time.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: TerkOS C++ Compiler (Cygwin)'
	/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/bin/g++ -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/include -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/usr/include -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/include/terk $(CPPFLAGS) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TerkOS C Compiler (Cygwin)'
	/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/bin/gcc -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/include -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/usr/include -I/usr/local/terkos/arm/arm-oe-linux-uclibcgnueabi/include/terk $(CFLAGS) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


