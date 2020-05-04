help:
	@echo "Usage:"
	@echo "make lib\tBuilds a shared library object for use in other projects"
	@echo "make libforce\tErases the current builds and rebuilds the library"
	@echo "make treetest\tBuild a test program for the individual trees (treeTest.c)"
	@echo "make foresttest\tBuild a test program for the forests (forestTest.c)"
	@echo "make all\tBuilds the shared library and all the test programs"
treetest:
	make lib
	gcc -o build/treetest -Lbuild/ -Wl,-rpath=./build src/treeTest.c -lm -Wall -Werror -g -ltreeclassifier
foresttest:
	make lib
	#gcc -o build/foresttest -Lbuild/ -Wl,-rpath=./build src/forestTest.c -lm -Wall -Werror -g -ltreeclassifier
	gcc -o build/foresttest src/forestTest.c src/treeClassifier.c -lm -Wall -Werror -g
libforce:
	rm -rf build
	make lib
lib:
	make build/libtreeclassifier.so
build/libtreeclassifier.so:
	@if [ -d build ];then rm -rf build;fi
	@mkdir build
	gcc -Wall -Werror -lm -fpic -c -o build/libtreeclassifier.o src/treeClassifier.c
	gcc -shared -o build/libtreeclassifier.so build/libtreeclassifier.o -lm -Wall -Werror
	rm build/*.o
all:
	@make libforce
	@make treetest
	@make foresttest