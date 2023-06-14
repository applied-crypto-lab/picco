#Add inputs and outputs to the build variables
CPP_SRCS += \
../smc-compute/ops/shamir/BitLTC.cpp \
../smc-compute/ops/shamir/LTZ.cpp \
../smc-compute/ops/shamir/Mod2.cpp \
../smc-compute/ops/shamir/Mod2M.cpp \
../smc-compute/ops/shamir/Mod2MS.cpp \
../smc-compute/ops/shamir/Mult.cpp \
../smc-compute/ops/shamir/Operation.cpp \
../smc-compute/ops/shamir/EQZ.cpp \
../smc-compute/ops/shamir/PrefixMultiplication.cpp \
../smc-compute/ops/shamir/SDiv.cpp \
../smc-compute/ops/shamir/Trunc.cpp \
../smc-compute/ops/shamir/BitOps.cpp \
../smc-compute/ops/shamir/DotProduct.cpp \
../smc-compute/ops/shamir/Pow2.cpp \
../smc-compute/ops/shamir/BitDec.cpp \
../smc-compute/ops/shamir/AddBitwise.cpp \
../smc_compute/ops/shamir/PreOpL.cpp \
../smc-compute/ops/shamir/B2U.cpp \
../smc-compute/ops/shamir/PreOr.cpp \
../smc-compute/ops/shamir/TruncS.cpp \
../smc-compute/ops/shamir/Inv.cpp \
../smc-compute/ops/shamir/TruncPr.cpp \
../smc-compute/ops/shamir/FLMult.cpp \
../smc-compute/ops/shamir/FLLTZ.cpp \
../smc-compute/ops/shamir/FLDiv.cpp \
../smc-compute/ops/shamir/FLAdd.cpp \
../smc-compute/ops/shamir/FLEQZ.cpp \
../smc-compute/ops/shamir/FLRound.cpp \
../smc-compute/ops/shamir/PrivIndex.cpp \
../smc-compute/ops/shamir/Random.cpp \
../smc-compute/ops/shamir/PrviPtr.cpp \
../smc-compute/ops/shamir/Int2FL.cpp \
../smc-compute/ops/shamir/FL2Int.cpp \
../smc-compute/ops/shamir/FPDiv.cpp \
../smc-compute/ops/shamir/AppRcr.cpp \
../smc-compute/ops/shamir/IntAppRcr.cpp \
../smc-compute/ops/shamir/Open.cpp \
../smc-compute/ops/shamir/Norm.cpp 

OBJS += \
./smc-compute/ops/shamir/BitLTC.o \
./smc-compute/ops/shamir/LTZ.o \
./smc-compute/ops/shamir/Mod2.o \
./smc-compute/ops/shamir/Mod2M.o \
./smc-compute/ops/shamir/Mod2MS.o \
./smc-compute/ops/shamir/Mult.o \
./smc-compute/ops/shamir/Operation.o \
./smc-compute/ops/shamir/EQZ.o \
./smc-compute/ops/shamir/PrefixMultiplication.o \
./smc-compute/ops/shamir/SDiv.o \
./smc-compute/ops/shamir/Trunc.o \
./smc-compute/ops/shamir/BitOps.o \
./smc-compute/ops/shamir/DotProduct.o \
./smc-compute/ops/shamir/Pow2.o \
./smc-compute/ops/shamir/BitDec.o \
./smc-compute/ops/shamir/AddBitwise.o \
./smc-compute/ops/shamir/PreOpL.o \
./smc-compute/ops/shamir/B2U.o \
./smc-compute/ops/shamir/PreOr.o \
./smc-compute/ops/shamir/TruncS.o \
./smc-compute/ops/shamir/Inv.o \
./smc-compute/ops/shamir/TruncPr.o \
./smc-compute/ops/shamir/FLMult.o \
./smc-compute/ops/shamir/FLLTZ.o \
./smc-compute/ops/shamir/FLDiv.o \
./smc-compute/ops/shamir/FLAdd.o \
./smc-compute/ops/shamir/FLEQZ.o \
./smc-compute/ops/shamir/FLRound.o \
./smc-compute/ops/shamir/PrivIndex.o \
./smc-compute/ops/shamir/Random.o \
./smc-compute/ops/shamir/Int2FL.o \
./smc-compute/ops/shamir/FL2Int.o \
./smc-compute/ops/shamir/IntDiv.o \
./smc-compute/ops/shamir/PrivPtr.o \
./smc-compute/ops/shamir/FPDiv.o \
./smc-compute/ops/shamir/AppRcr.o \
./smc-compute/ops/shamir/IntAppRcr.o \
./smc-compute/ops/shamir/Open.o \
./smc-compute/ops/shamir/Norm.o 


CPP_DEPS += \
./smc-compute/ops/shamir/BitLTC.d \
./smc-compute/ops/shamir/LTZ.d \
./smc-compute/ops/shamir/Mod2.d \
./smc-compute/ops/shamir/Mod2M.d \
./smc-compute/ops/shamir/Mod2MS.d \
./smc-compute/ops/shamir/Mult.d \
./smc-compute/ops/shamir/Operation.d \
./smc-compute/ops/shamir/EQZ.d \
./smc-compute/ops/shamir/PrefixMultiplication.d \
./smc-compute/ops/shamir/SDiv.d \
./smc-compute/ops/shamir/Trunc.d \
./smc-compute/ops/shamir/BitOps.d \
./smc-compute/ops/shamir/DotProduct.d \
./smc-compute/ops/shamir/Pow2.d \
./smc-compute/ops/shamir/BitDec.d \
./smc-compute/ops/shamir/AddBitwise.d \
./smc-compute/ops/shamir/PreOpL.d \
./smc-compute/ops/shamir/B2U.d \
./smc-compute/ops/shamir/PreOr.d \
./smc-compute/ops/shamir/TruncS.d \
./smc-compute/ops/shamir/Inv.d \
./smc-compute/ops/shamir/TruncPr.d \
./smc-compute/ops/shamir/FLLTZ.d \
./smc-compute/ops/shamir/FLMult.d \
./smc-compute/ops/shamir/FLDiv.d \
./smc-compute/ops/shamir/FLAdd.d \
./smc-compute/ops/shamir/FLEQZ.d \
./smc-compute/ops/shamir/FLRound.d \
./smc-compute/ops/shamir/PrivIndex.d \
./smc-compute/ops/shamir/Random.d \
./smc-compute/ops/shamir/Int2FL.d \
./smc-compute/ops/shamir/FL2Int.d \
./smc-compute/ops/shamir/IntDiv.d \
./smc-compute/ops/shamir/PrivPtr.d \
./smc-compute/ops/shamir/FPDiv.d \
./smc-compute/ops/shamir/AppRcr.d \
./smc-compute/ops/shamir/IntAppRcr.d \
./smc-compute/ops/shamir/Open.d \
./smc-compute/ops/shamir/Norm.d  
 
#Rules for building source in ops
smc-copmpute/ops%.o: ../smc-compute/ops/shamir/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished Building $<'
	@Echo ' '
 

