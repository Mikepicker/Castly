#OBJS specifies which files to compile as part of the project
OBJS = main.cpp

#CC specifies which compiler we're using
CC = g++ -std=c++14 -D LINUX
MINGW = i686-w64-mingw32-g++ -std=c++14 -I ../SDL2-2.0.7-mingw/i686-w64-mingw32/include/SDL2 -I ../SDL2_image-2.0.2/i686-w64-mingw32/include -I ../SDL2_ttf-2.0.14/i686-w64-mingw32/include -D WIN

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
#  COMPILER_FLAGS = -w
#
#  #LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
WIN_LINKER_FLAGS = -mwindows -L ../SDL2-2.0.7-mingw/i686-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -L ../SDL2_image-2.0.2/i686-w64-mingw32/lib -lSDL2_image -L ../SDL2_ttf-2.0.14/i686-w64-mingw32/lib -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = castly
EXE_NAME = castly.exe

#This is the target that compiles our executable
all : $(OBJS)
		$(CC) -g $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

linux : $(OBJS)
		$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME) && ./bundle_linux.sh

win : $(OBJS)
	$(MINGW) $(OBJS) $(COMPILER_FLAGS) $(WIN_LINKER_FLAGS) -o $(EXE_NAME)
