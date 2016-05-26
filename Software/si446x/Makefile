#############################################################
# Makefile for execute.
#############################################################
#set your own environment option
CC = arm-linux-gcc
CXX = arm-linux-g++
CFLAG = -D_NOMNG -D_FILELINE  -O2 -static
CXXFLAG = -static

#set your inc and lib
INC = 
LIB = 

PRG = si446x
OBJ = gpio.o \
	spidev.o \
	si446x.o \
	buffer.o \
	main.o

$(PRG):$(OBJ)
	$(CC) $(CFLAG) $(INC) -o $@ $(OBJ) $(LIB)
	
.SUFFIXES: .c .o .cpp
.c.o:
	$(CC) -c $(CFLAG) $(INC) $*.c -o $*.o
.cpp.o:
	$(CXX) -c $(CFLAG) $(INC) $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing execute and compiled files......"
	rm -f $(OBJ) $(PRG)
