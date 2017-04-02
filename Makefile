# Makefile for t1.cpp
#OBJS specifies which files to compile as part of the project
OBJS = navDisplay.cpp RGBpixmap.cpp 

#CC specifies which compiler we're using
CC = g++

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = navDisplay 

#This is the target that compiles our executable
all : navDisplay 

navDisplay : navDisplay.cpp
	g++ navDisplay.cpp RGBpixmap.cpp -o navDisplay -lGL -lGLU -lglut -lSOIL

clean:
	rm *o navDisplay
