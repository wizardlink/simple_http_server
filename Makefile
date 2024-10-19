CODE := $(wildcard *.c) $(wildcard */*.c)

all: $(CODE)
	mkdir -p bin
	$(CC) $(CODE) -o bin/server

clean:
ifneq ("$(wildcard bin)", "")
	rm -rf bin
else
	echo "File already deleted."
endif
