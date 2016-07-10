CPP			    = g++
RM			    = rm -f
PLAT_FLAG		= -m32
CPPFLAGS		= -Wall -c -I. -I.. -I../amqpcpp/include -I../amqpcpp/src -flto -std=c++11 -g
LD			    = g++
LD_FLAGS		= -Wall -shared -O2 -L../amqpcpp/src -lamqpcpp -L../network -lnetwork
SHARED_LIB		= libamqpclient.so
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
	#cp -f *.h ${INCLUDE_DIR}/amqpcpp
	#cp -f lib/* ${LIBRARY_DIR}
	cp -f ${SHARED_LIB} ${LIBRARY_DIR}
	cp -f ${STATIC_LIB} ${LIBRARY_DIR}

${SHARED_OBJECTS}:${SOURCES}
	${CPP} ${PLAT_FLAG} ${CPPFLAGS} -fpic -o $@ ${@:%.o=%.cpp}

${STATIC_OBJECTS}:${SOURCES}
	${CPP} ${PLAT_FLAG} ${CPPFLAGS} -o $@ ${@:%.s.o=%.cpp}

