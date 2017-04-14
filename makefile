all: 
	lex sim.l
	yacc -d sim.y
	gcc -O3 -Wall -o processor_cache_simulator sim.h cache.h y.tab.c lex.yy.c sim.c pipeline.c printsvg.c cache.c -lm -pthread
	rm *.tab.* *.yy.*
clean:
	rm a.out *.svg results*.txt