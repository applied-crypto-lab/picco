# All of the sources participating in the build are defined here
-include smc-compute/ops/shamir/ops.mk
-include smc-compute/smc-compute.mk
-include objects.mk

#Add test program to the build variables
CPP_SRCS += \
#test_op.cpp 

OBJS += \
#test_op.o 

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif


# Add inputs and outputs from these tool invocations to the build variables 
# All Target
sorted-ll: $(OBJS) $(USER_OBJS) 
	g++ -c -w sorted-ll.cpp 
	g++ -o sorted-ll sorted-ll.o $(OBJS) $(USER_OBJS) $(LIBS) 
# replace the line above with this one for multi-threaded programs
#	g++ -o sorted-ll sorted-ll.o -I ./ompi/include/ompi $(OBJS) $(USER_OBJS) $(LIBS) -L ./ompi/lib/ompi -L ./ompi/lib/ompi/default -lort -lrt -lboost_thread 

clean:
	-$(RM) *.o sorted-ll 

#Clean-Everything
clean-all:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) *.o sorted-ll 

