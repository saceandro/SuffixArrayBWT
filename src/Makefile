SRCS := $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
TARGETS = larssonSadakane3 inducedSort4 randArray inducedSort4Check inducedSort4time burrowsWheeler2 burrowsWheeler2time
RM = rm -f

all: $(TARGETS)
%: %.cc
	g++ -O3 -Wall -g -o $@ $<
clean:
	$(RM) $(OBJS) $(TARGETS)
check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
.PHONY: clean check-syntax
