gcc -Wall -pedantic -ansi -c analyzer.c
gcc -Wall -pedantic -ansi -c utils.c
gcc -Wall -pedantic -ansi -c file_manager.c
gcc -Wall -pedantic -ansi -c linked_list.c
gcc -Wall -pedantic -ansi -c encoder.c
gcc -Wall -pedantic -ansi -c second_pass.c
gcc -Wall -pedantic -ansi -o Prog.exe analyzer.o utils.o file_manager.o linked_list.o encoder.o second_pass.o
