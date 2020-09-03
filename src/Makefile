XX = g++

CXXFLAGS =  -c  -g -std=c++11
CLIBS =  -lpthread 
#-luuid -ldl -lssl -lcrypto
#INCLUDE_DIRS = -I../../include  

SRC=$(wildcard *.cpp)
OBJECTS:=$(patsubst %.cpp,%.o,$(SRC))

PROGRAM:= srs_ingest_helper
BINDIR := ./
TARGET = $(BINDIR)/$(PROGRAM)



all: MD $(TARGET)  


$(TARGET) : $(OBJECTS) 
	$(XX) -static-libstdc++  -o $(TARGET)  $(OBJECTS)  $(CLIBS)  


$(OBJECTS) : %.o : %.cpp 
	$(XX)  $(CXXFLAGS) $< -o $@ $(INCLUDE_DIRS) 

MD:
	mkdir -p  $(BINDIR)


	
	

.PHONY : clean

clean:
	rm -rf $(TARGET) $(OBJECTS)

