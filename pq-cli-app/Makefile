uname := $(shell uname)

ifeq ($(findstring $(uname), Linux GNU GNU/kFreeBSD), $(uname))
install: pq.c	
	gcc -o pq pq.c
	strip pq
clean: 
	rm -f pq
endif

ifeq ($(uname), Darwin)
install: pq.c
	gcc -o pq pq.c
	strip pq
clean: 
	rm -f pq
endif

ifeq ($(filter MINGW% MSYS%, $(uname)), $(uname))
install: pq.c	
	gcc -o pq pq.c
	strip pq.exe
clean: 
	rm -f pq.exe	
endif