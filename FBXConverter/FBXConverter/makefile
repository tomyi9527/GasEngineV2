# make build=release
# make build=debug
VARIANT := $(if $(build),$(build),release)
SUFFIX := _release
ifeq "$(VARIANT)" "debug"
GCC_DEBUG_FLAGS += -g -D_DEBUG
SUFFIX := _debug
$(info ************ BUILD IN DEBUG MODE ************)
else
$(info ============ BUILD IN RELEASE MODE ============)
endif


LIB=\
-L/usr/lib/gcc4/x64/$(VARIANT)

INC=\
-I/usr/include \
-I./ \
-I./zlib-1.2.8 \
-I./TootleLib \
-I./TootleLib/include \
-I./TootleLib/RayTracer \
-I./TootleLib/RayTracer/Math \
-I./TootleLib/RayTracer/JRT \
-I./tristripper \
-I./tristripper/include \
-I./NvTriStrip \
-I./libtga-1.0.1 \
-I./TexturePipeline \
-I./lpng1627 \
-I./optimization/GAS_V1 \
-I./optimization/GAS_V2 \
-I./optimization/GLTF_V2 \
-I./optimization \
-I./MAX/compoundfilereader/src/include \
-I./MAX/glm-0.9.9.8/glm \
-I./MAX/src \
-I./MAX

CC = gcc
XX = g++
CFLAGS = -Wall -O2  $(INC)
# CFLAGS = -Wall -O -g $(INC)
TARGETDIR := ../../build/FBXConverter

TARGET = ${TARGETDIR}${SUFFIX}

CXXFLAGS = -std=c++17
CFLAGS += $(GCC_DEBUG_FLAGS)

OBJDIR  = ./obj/${VARIANT}/

all : obj_dir $(TARGET) 

SOURCES = \
$(wildcard *.c *.cpp *.cxx) \
$(wildcard Common/*.c Common/*.cpp Common/*.cxx) \
$(wildcard cJSON/*.c)  \
$(wildcard zlib-1.2.8/*.c) \
$(wildcard lpng1627/*.c) \
$(wildcard jpeg-6b/*.c) \
$(wildcard libtga-1.0.1/*.c) \
$(wildcard NvTriStrip/*.cpp) \
$(wildcard TexturePipeline/*.cpp) \
$(wildcard tristripper/*.cpp tristripper/*.c tristripper/*.cxx) \
$(wildcard TootleLib/*.cpp TootleLib/*.c TootleLib/*.cxx) \
$(wildcard TootleLib/RayTracer/*.cpp TootleLib/RayTracer/*.c TootleLib/RayTracer/*.cxx) \
$(wildcard TootleLib/RayTracer/JRT/*.cpp TootleLib/RayTracer/JRT/*.c TootleLib/RayTracer/JRT/*.cxx) \
$(wildcard TootleLib/RayTracer/Math/*.cpp TootleLib/RayTracer/Math/*.c TootleLib/RayTracer/Math/*.cxx) \
$(wildcard MMD/*.cpp) \
$(wildcard STL/*.cpp) \
$(wildcard optimization/GAS_V1/*.cpp) \
$(wildcard optimization/GAS_V2/*.cpp) \
$(wildcard optimization/GLTF_V2/*.cpp) \
$(wildcard optimization/*.cpp) \
$(wildcard MAX/src/*.cpp) \
$(wildcard MAX/*.cpp)


OBJS = $(addprefix ${OBJDIR},$(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(patsubst %.cxx, %.o, $(SOURCES)))))
#$(warning $(SOURCES))

LIBDIR = /usr/lib/gcc4/x64/$(VARIANT)
FBXSDK_LIB = $(LIBDIR)/libfbxsdk.a
LIBS = $(FBXSDK_LIB) -lm -lrt -luuid -lstdc++ -lpthread -ldl

$(TARGET) : $(OBJS) 
	$(XX) $(OBJS)  $(LIB) $(LIBS) -o $(TARGET)
	chmod a+x $(TARGET)

$(OBJDIR)%.o : %.c 
	$(CC) $(CFLAGS) -c $< -o $@
	
${OBJDIR}%.o : %.cpp 
	$(XX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@
	
${OBJDIR}%.o : %.cxx 
	$(XX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

obj_dir : 
	mkdir -p $(OBJDIR)Common
	mkdir -p $(OBJDIR)cJSON
	mkdir -p $(OBJDIR)zlib-1.2.8
	mkdir -p $(OBJDIR)lpng1627
	mkdir -p $(OBJDIR)jpeg-6b
	mkdir -p $(OBJDIR)libtga-1.0.1
	mkdir -p $(OBJDIR)NvTriStrip
	mkdir -p $(OBJDIR)TootleLib
	mkdir -p $(OBJDIR)TootleLib/RayTracer
	mkdir -p $(OBJDIR)TootleLib/RayTracer/JRT
	mkdir -p $(OBJDIR)TootleLib/RayTracer/Math
	mkdir -p $(OBJDIR)tristripper
	mkdir -p $(OBJDIR)TexturePipeline
	mkdir -p $(OBJDIR)MMD
	mkdir -p $(OBJDIR)STL
	mkdir -p $(OBJDIR)optimization
	mkdir -p $(OBJDIR)optimization/GAS_V1
	mkdir -p $(OBJDIR)optimization/GAS_V2
	mkdir -p $(OBJDIR)optimization/GLTF_V2
	mkdir -p $(OBJDIR)MAX/src

.PHONY : obj_dir
	
clean:
	rm -rf $(TARGET) 
	rm -rf ${OBJDIR} 
