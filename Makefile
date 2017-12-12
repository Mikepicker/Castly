#OBJS specifies which files to compile as part of the project
OBJS = main.cpp

#CC specifies which compiler we're using
CC = g++ -std=c++14 -g
MINGW = i686-w64-mingw32-g++ -std=c++14 -I ./SDL2-2.0.7-mingw/i686-w64-mingw32/include/SDL2 -I ./SDL2_image-2.0.2/i686-w64-mingw32/include -I ./SDL2_ttf-2.0.14/i686-w64-mingw32/include

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
#  COMPILER_FLAGS = -w
#
#  #LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
WIN_LINKER_FLAGS = -mwindows -L ./SDL2-2.0.7-mingw/i686-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -L ./SDL2_image-2.0.2/i686-w64-mingw32/lib -lSDL2_image -L ./SDL2_ttf-2.0.14/i686-w64-mingw32/lib -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = main
EXE_NAME = main.exe

#This is the target that compiles our executable
all : $(OBJS)
		$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

win : $(OBJS)
	$(MINGW) $(OBJS) $(COMPILER_FLAGS) $(WIN_LINKER_FLAGS) -o $(EXE_NAME)
