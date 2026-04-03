SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)

release: $(SOURCES) $(HEADERS)
	@mkdir -p bin
	gcc -O3 -msse4 -lpthread -fopenmp -Wall $(SOURCES) -o bin/conv

debug: $(SOURCES) $(HEADERS)
	@mkdir -p bin
	gcc -O0 -g -DDEBUG -msse4 -lpthread -fopenmp -Wall $(SOURCES) -o bin/conv

clean:
	@rm -rf bin