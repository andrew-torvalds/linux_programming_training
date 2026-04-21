FILE_CODE := main.c
FILE_EXE  := main

all: 
	gcc $(FILE_CODE) -o $(FILE_EXE)
	gcc worker.c -o worker

clean:
	rm -rf $(FILE_EXE)