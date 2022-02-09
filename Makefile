rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CC = gcc
CFLAGS = -O3 -g -Wall

INCLUDES = -I/usr/include/glib-2.0 -Iinclude
LIBS = -lm -lpthread -lglib-2.0 -lncursesw
PKG_CONFIG = `pkg-config --cflags --libs glib-2.0`

HEADERS = $(call rwildcard,include,*.h)
SRC = $(call rwildcard,src,*.c)
OBJS = ${SRC:src/%.c=obj/%.o}

.PHONY: default
default: guiao-3

.PHONY: clean
clean:
	rm -f ${OBJS} core *.core guiao-3 test saida/*.indx saida/*.dat saida/*.tmp saida/*.txt


obj/%.o: src/%.c ${HEADERS}
	mkdir -p $(dir $@)
	${CC} ${CFLAGS} -c -o $@ ${INCLUDES} ${PKG_CONFIG} $< ${LIBS}


guiao-3: $(filter-out obj/tests/tests.o,$(OBJS))
	${CC} ${CFLAGS} -o $@ ${INCLUDES} ${PKG_CONFIG} $^ ${LIBS}

test: $(filter-out obj/main.o,$(OBJS))
	${CC} ${CFLAGS} -o $@ ${INCLUDES} ${PKG_CONFIG} $^ ${LIBS}
