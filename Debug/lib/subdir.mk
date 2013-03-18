################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/HT.c \
../lib/core.c \
../lib/linked_list.c \
../lib/query.c \
../lib/trie.c \
../lib/word.c 

OBJS += \
./lib/HT.o \
./lib/core.o \
./lib/linked_list.o \
./lib/query.o \
./lib/trie.o \
./lib/word.o 

C_DEPS += \
./lib/HT.d \
./lib/core.d \
./lib/linked_list.d \
./lib/query.d \
./lib/trie.d \
./lib/word.d 


# Each subdirectory must supply rules for building sources it contributes
lib/%.o: ../lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


