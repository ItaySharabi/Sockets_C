GCC = gcc -Wall -g

all: Sender Measure

Sender: sender.o measure.o
	$(GCC) -o Sender send
Measure: measure.o sender.o
	$(GCC) -o Measure get
measure.o: measure.c
	$(GCC) -c measure.c -o get
sender.o: sender.c
	$(GCC) -c sender.c -o send
.PHONY: clean.
clean:
	rm -f *.o *.so Measure Sender send get