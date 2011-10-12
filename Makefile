
srcdir = .
LDFLAGS =  -Wl,-elf2flt="-r -z -s 32768"
LIBS =  
CFLAGS = -O2 -fomit-frame-pointer -fno-builtin -fpic -msingle-pic-base

# Change these if necessary

CC = arm-elf-gcc 
CPP = arm-elf-gcc -E

all:	BFR

BFR:
	$(CCP) -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS) $@.cpp
	
clean:
	rm -f $(OBJS) BFR *.gdb   
	


