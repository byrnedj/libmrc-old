.SUFFIXES: .c .o
CC = gcc
EXEC = gen_mrc
CCFLAGS = -O0 -g -I ../include -L ../ -lmrc -Wl,-rpath,../
OBJS = gen_mrc.o 

${EXEC}: ${OBJS}
	${CC} ${CCFLAGS} -o ${EXEC} ${OBJS}
	rm -f ${OBJS}

.c.o:
	${CC} ${CCFLAGS} -c $<


run: ${EXEC}
	./${EXEC}
    
clean:
	rm -f ${EXEC} ${OBJS}

gen_mrc.o: gen_mrc.c 

