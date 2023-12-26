VARIANT = release
TARGETDIR := ../build/compiler
SUFFIX := _release
ifeq "$(VARIANT)" "debug"
	GCC_DEBUG_FLAGS += -g -D_DEBUG
	SUFFIX := _debug
endif

CC=g++
CXXFLAGS = $(GCC_DEBUG_FLAGS)

TARGET =  ${TARGETDIR}${SUFFIX}

SRCS = compiler.cpp

OBJS = $(SRCS:.cpp=.o)

start:$(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

.cpp.o:
	$(CC) $(CXXFLAGS) -o $@ -c $< 

RELEASE := _release
DEBUG := _debug
clean:
	rm -rf $(OBJS)
	rm -rf ${TARGETDIR}${RELEASE}
	rm -rf ${TARGETDIR}${DEBUG}