all: agent router
.PHONY: all

agent: agent.o
	gcc -o agent agent.o

router: router.o
	gcc -o router router.o

agent.o: routing_simulation.h agent.c
	gcc -c agent.c

router.o: routing_simulation.h router.c
	gcc -c router.c

.PHONY: clean
clean:
	rm -f agent router