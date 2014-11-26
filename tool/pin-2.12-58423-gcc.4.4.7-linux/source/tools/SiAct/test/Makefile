CC=g++
INCLUDE="-I../include"

all:  test.app1 test.app2 test.app3

test.%: %.o 
	$(CC) $(INCLUDE) -o $@ $<
	
%.o: %.cpp
	$(CC) $(COPTS) $(INCLUDE) -o $@ -c $< 
	
clean:
	rm -f *.o test.*
