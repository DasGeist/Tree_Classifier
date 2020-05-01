builds:
	if [ -d build ];then rm -rf build;fi
	mkdir build
	gcc -o build/treetest src/treeClassifier.c src/treeTest.c -lm -Wall -Werror -g