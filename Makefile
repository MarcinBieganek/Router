CC=g++
CFLAGS=-I. -std=gnu++11 -Wall -Wextra
DEPS = router.hpp
OBJ = router.o sys_calls.o ip_addr_fun.o time_fun.o udp_datagram.o vectors_op.o

%.o: %.cpp $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)
	
router: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
		rm -f *.o

distclean:
		rm -f *.o
		rm -f ./router

