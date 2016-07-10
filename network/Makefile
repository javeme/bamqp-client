#g++ -std=c++11 -m32 -I./include  -lACE_v6 *.cpp
# -pipe -fcheck-new -DACE_HAS_STANDARD_CPP_LIBRARY=1 -D_CPP -DACE_HAS_EXCEPTIONS -D_REENTRANT -DACE_HAS_AIO_CALLS -D__ACE_INLINE__=1 -DACE_NDEBUG=1 -DACE_NTRACE=1 -fPIC
CPP			    = g++
RM			    = rm -f
PLAT_FLAG		= -m32
CPPFLAGS		= -Wall -c -I. -I./include -I.. -flto -std=c++11 -g
LD			    = g++
LD_FLAGS		= -Wall -shared -O2 -L./lib -lACE_v6
SHARED_LIB		= libnetwork.so
STATIC_LIB		= $(SHARED_LIB:%.so=%.a)
SOURCES			= $(wildcard *.cpp)
SHARED_OBJECTS	= $(SOURCES:%.cpp=%.o)
STATIC_OBJECTS	= $(SOURCES:%.cpp=%.s.o)
PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib

all:	shared static

shared:	${SHARED_OBJECTS} ${SHARED_LIB}

static:	${STATIC_OBJECTS} ${STATIC_LIB}

${SHARED_LIB}: ${SHARED_OBJECTS}
	${LD} ${PLAT_FLAG} ${LD_FLAGS} -o $@ ${SHARED_OBJECTS}

${STATIC_LIB}: ${STATIC_OBJECTS}
	ar rcs ${STATIC_LIB} ${STATIC_OBJECTS}

clean:
	${RM} *.obj *~* ${SHARED_OBJECTS} ${STATIC_OBJECTS} ${SHARED_LIB} ${STATIC_LIB}
    
install:
	#cp -f include/* ${INCLUDE_DIR}
	cp -f lib/* ${LIBRARY_DIR}
	cp -f ${SHARED_LIB} ${LIBRARY_DIR}
	cp -f ${STATIC_LIB} ${LIBRARY_DIR}

${SHARED_OBJECTS}:${SOURCES}
	${CPP} ${PLAT_FLAG} ${CPPFLAGS} -fpic -o $@ ${@:%.o=%.cpp}

${STATIC_OBJECTS}:${SOURCES}
	${CPP} ${PLAT_FLAG} ${CPPFLAGS} -o $@ ${@:%.s.o=%.cpp}

