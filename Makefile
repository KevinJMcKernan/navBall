# Makefile for t1.cpp
#OBJS specifies which files to compile as part of the project
OBJS = navDisplay.cpp RGBpixmap.cpp

#CC specifies which compiler we're using
CC = g++

#LINKER_FLAGS specifies the libraries we're linking against
#LINKER_FLAGS = 

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = navDisplay

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(OBJ_NAME) -lGL -lGLU -lglut

clean:
	rm *o navDisplay
