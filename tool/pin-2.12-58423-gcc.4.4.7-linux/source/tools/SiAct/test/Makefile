CC=g++
INCLUDE="-I../include"
DEPS="../include/pin_util.o"

all: deps test.app1 test.app2 test.app3

deps:
	cd ../include; make
	cd ..; make
	
test.%: %.o
	$(CC) $(INCLUDE) -o $@ $(DEPS) $<
	
%.o: %.cpp
	$(CC) $(COPTS) $(INCLUDE) -o $@ -c $< 
	
clean:
	rm -f *.o test.*
