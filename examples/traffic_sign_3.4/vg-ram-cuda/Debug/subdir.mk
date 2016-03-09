################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vgram_files.c 

CU_SRCS += \
../vgram_main.cu 

OBJS += \
./vgram_files.o \
./vgram_main.o 

C_DEPS += \
./vgram_files.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: NVIDIA CUDA Compiler'
	nvcc -I/usr/local/cuda/include -I/usr/local/cuda/SDK/C/common/inc -O0 -g -pg -c -Xcompiler -fmessage-length=0 -arch compute_13 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


