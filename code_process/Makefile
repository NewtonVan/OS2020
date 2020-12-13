objects= main.o shell.o RCB.o PCB.o
CXX= g++
CXXFLAGS= -std=c++11 -Wall -Werror -I ./lib
vpath %.cpp src
vpath %.h lib

define GetObj
$(CXX) $(CXXFLAGS) -c -o $@ $<
endef

test_shell.exe : $(objects)
	@$(CXX) $(CXXFLAGS) -o $@ $(objects) 
	@del $(objects)

main.o: main.cpp shell.h RCB.h hlibrary.h PCB.h
	@$(GetObj)

PCB.o : PCB.cpp PCB.h hlibrary.h	
	@$(GetObj)

RCB.o : RCB.cpp RCB.h hlibrary.h
	@$(GetObj)

shell.o : shell.cpp shell.h RCB.h hlibrary.h PCB.h
	@$(GetObj)

.PHONY: clean cleanAll
clean :
	@del $(objects)
cleanAll :
	@del $(objects) *.exe