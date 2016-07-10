PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib

all:
		$(MAKE) -C amqpcpp all
		$(MAKE) -C network all
		$(MAKE) -C mq all

static:
		$(MAKE) -C amqpcpp static
		$(MAKE) -C network static
		$(MAKE) -C mq static

shared:
		$(MAKE) -C amqpcpp shared
		$(MAKE) -C network shared
		$(MAKE) -C mq shared

clean:
		$(MAKE) -C amqpcpp clean
		$(MAKE) -C network clean
		$(MAKE) -C mq clean

install:
		$(MAKE) -C amqpcpp install
		$(MAKE) -C network install
		$(MAKE) -C mq install
