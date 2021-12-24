TARGETS=timer_test
LIBRARY_HDR=timer.hpp
LIBRARY_SRC=

build/x86:
	mkdir -p $@

all:
	make $(TARGETS)

%:%.cpp $(LIBRARY_SRC) $(LIBRARY_HDR) Makefile build/x86
	g++ $< $(LIBRARY_SRC) -o build/x86/$@ -I. -std=c++11 -O0 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

clean:
	rm -rvf build