
#To run, put this file together with lexer.h, and lexer.c
#in the same directory. Run "make". Then the executable
#is "schemer," which just takes a line of input and
#breaks it up into tokens.

schemer: structuraltester.o lexer.o evaluation.o parser.o
	gcc -o schemer structuraltester.o lexer.o evaluation.o parser.o

structuraltester.o: structuraltester.c
	gcc -c structuraltester.c

lexer.o: lexer.c
	gcc -c lexer.c

evaluation.o: evaluation.c
	gcc -c evaluation.c
	
parser.o: parser.c
	gcc -c parser.c

clean:
	rm -f *~ *.o *.a

#^^^^^^This space must be a TAB!!.

