release: src/conv-harness.c src/conv-openmp.c src/conv-openmp.h src/conv-pthread.c src/conv-pthread.h
	@mkdir -p bin
	gcc -O3 -msse4 -lpthread -fopenmp -Wall src/conv-harness.c src/conv-openmp.c src/conv-pthread.c -o bin/conv

debug: src/conv-harness.c src/conv-openmp.c src/conv-openmp.h src/conv-pthread.c src/conv-pthread.h
	@mkdir -p bin
	gcc -O0 -g -DDEBUG -msse4 -lpthread -fopenmp -Wall src/conv-harness.c src/conv-openmp.c src/conv-pthread.c -o bin/conv

clean:
	@rm -rf bin