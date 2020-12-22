objects= main.o shell.o RCB.o PCB.o
CXX= g++
CXXFLAGS= -std=c++11 -Wall -Werror -I ./lib
vpath %.cpp src
vpath %.h lib

test_shell.exe : $(objects)
	@$(CXX) $(CXXFLAGS) -o $@ $(objects) 

main.o : shell.h RCB.h hlibrary.h PCB.h
PCB.o : PCB.h hlibrary.h	
RCB.o : RCB.h hlibrary.h
shell.o : shell.h RCB.h hlibrary.h PCB.h

.PHONY: cleanall cleanobj cleanexe
cleanall : cleanobj cleanexe
cleanobj :
	@del $(objects)
cleanexe :
	@del *.exe