COPTS=-Wall -pedantic -O4

sim : main.o mem.o bus.o io.o cpu.o term.o
	gcc -o sim main.o mem.o bus.o io.o cpu.o term.o

main.o : main.c mem.h bus.h cpu.h io.h
	gcc -c main.c $(COPTS)

mem.o : mem.c mem.h
	gcc -c mem.c $(COPTS)

io.o : io.c io.h
	gcc -c io.c $(COPTS)

cpu.o : cpu.c cpu.h
	gcc -c cpu.c $(COPTS)

bus.o : bus.c bus.h
	gcc -c bus.c $(COPTS)

term.o : term.c term.h
	gcc -c term.c $(COPTS)

clean : 
	rm *.o sim
