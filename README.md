# A simple entropy-based forest classifier

This is an implementation of a few decision-tree building methods and a "bagging-like" meta-algorithm for generating easy-to-use forest classifiers.

## Building

### Building the test files

Assuming you have the standard c development environment (standard libraries - stdio.h, stdlib.h, string.h and math.h - and gcc) and your system supports the `make` command, simply open the root directory on your terminal and run `make test`.

Otherwise, run `gcc -o treetest src/treeClassifier.c src/treeTest.c -lm -Wall -Werror -g`.

### Building a shared object file for use in other projects

Again, assuming you have `make`, run `make lib`.

Otherwise, run `gcc -o libtreeclassifier.so src/treeClassifier.c -fPIC`

For building with this library, you may install it at your system's standard library path _or_ add the `-Wl,-rpath=<path-to-treeclassifier>/build` and `-ltreeclassifier` (at the end) options to your compiler (if you're using gcc).

Example:

This is the command used for building the test program: `gcc -o build/treetest -Lbuild/ -Wl,-rpath=./build src/treeTest.c -lm -Wall -Werror -g -ltreeclassifier`

_Made with <3 by Sérgio F._