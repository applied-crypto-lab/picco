#ADD inputs and output to the build variables
CPP_SRCS += \
../smc-compute/NodeConfiguration.cpp \
../smc-compute/NodeNetwork.cpp \
../smc-compute/SecretShare.cpp \
../smc-compute/ShamirUtil.cpp \
../smc-compute/ShamirOps.cpp \
../smc-compute/SMC_Utils.cpp

OBJS += \
./smc-compute/NodeConfiguration.o \
./smc-compute/NodeNetwork.o \
./smc-compute/SecretShare.o \
./smc-compute/ShamirUtil.o \
./smc-compute/ShamirOps.o \
./smc-compute/SMC_Utils.o

CPP_DEPS += \
./smc-compute/NodeConfiguration.d \
./smc-compute/NodeNetwork.d \
./smc-compute/SecretShare.d \
./smc-compute/ShamirUtil.d \
./smc-compute/ShamirOps.d \
./smc-compute/SMC_Utils.d

#Supply the rules for building the source
smc-compute/%.o: ../smc-compute/%.cpp
	@echo 'Building file:'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
                    

