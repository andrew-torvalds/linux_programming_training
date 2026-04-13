LOGGER_FILE_CODE := logger.c
LOGGER_FILE_EXE  := logger
LOG_DB_FILE		 := sensor.log

all: 
	gcc $(LOGGER_FILE_CODE) -o $(LOGGER_FILE_EXE)

clean:
	rm -rf $(LOGGER_FILE_EXE)

clslog:
	rm -rf $(LOG_DB_FILE)