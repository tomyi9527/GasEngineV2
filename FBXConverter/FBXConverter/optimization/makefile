LIB=-L/mnt/share/fbxsdk20161/lib/gcc4/x64/release
INC=\
-I/mnt/share/fbxsdk20161/include \
-I./ \
-I../ \
-I../zlib-1.2.8 \
-I../TootleLib \
-I../TootleLib/include \
-I../TootleLib/RayTracer \
-I../TootleLib/RayTracer/Math \
-I../TootleLib/RayTracer/JRT \
-I../tristripper \
-I../tristripper/include \
-I../NvTriStrip \
-I../libtga-1.0.1 \
-I../TexturePipeline \
-I../lpng1627 \
-I../adaptiveArithmeticCoding

#LIB=-L/usr/lib/gcc4/x64/release
#INC=\
#-I/usr/include \
#-I./ \
#-I./zlib-1.2.8 \
#-I./TootleLib \
#-I./TootleLib/include \
#-I./TootleLib/RayTracer \
#-I./TootleLib/RayTracer/Math \
#-I./TootleLib/RayTracer/JRT \
#-I./tristripper \
#-I./tristripper/include \
#-I./NvTriStrip \
#-I./libtga-1.0.1 \
#-I./TexturePipeline \
#-I./lpng1627 \
#-I./optimization

CC = gcc
XX = g++
YY = ar
CFLAGS = -Wall -O2 $(INC)
# CFLAGS = -Wall -O -g $(INC)
TARGET = ./liboptimization.a

%.o : %.c obj_dir
	$(CC) $(CFLAGS) -c $< -o ./obj/$@
	
%.o : %.cpp obj_dir
	$(XX) $(CFLAGS) -c $< -o ./obj/$@
	
%.o : %.cxx obj_dir
	$(XX) $(CFLAGS) -c $< -o ./obj/$@

SOURCES = \
FBXAnimationExporter_V4.cpp FBXSceneStructureExporter_V4.cpp JsonToBin_V4.cpp NeonateVertexCompression_V4.cpp

OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(patsubst %.cxx, %.o, $(SOURCES))))
#$(warning $(SOURCES))

$(TARGET) : $(OBJS)
	$(YY) rcs $(TARGET) $(addprefix ./obj/,$(OBJS))

obj_dir : 
	mkdir -p ./obj
	
clean:
	rm -rf $(TARGET) 
	rm -rf ./obj
