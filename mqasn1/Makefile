CPP				= g++
PLAT_FLAG		= -m32
CPPFLAGS		= -Wall -c -I. -I./include -I./include/snacc -I.. -O2 -flto -std=c++11 -g
LD				= g++
LDFLAGS			= -lACE_v6 -lnetwork -lamqpcpp -lamqpclient -L./lib -lc++asn1 
RESULT			= mq-asn1
SOURCES			= $(wildcard *.cpp)
OBJECTS			= $(SOURCES:%.cpp=%.o)
PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib

all:	${OBJECTS} ${RESULT}

${RESULT}: ${OBJECTS}
	${LD} ${PLAT_FLAG} -o $@ ${OBJECTS} ${LDFLAGS}

clean:
	${RM} *.obj *~* ${OBJECTS} ${RESULT}

install:
	cp -f lib/* ${LIBRARY_DIR}

${OBJECTS}:${SOURCES}
	${CPP} ${PLAT_FLAG} ${CPPFLAGS} -o $@ ${@:%.o=%.cpp}
