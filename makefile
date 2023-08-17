CC = gcc
CFLAGS = -Wall -Wextra

all: producer consumer

producer: producer.c
	$(CC) $(CFLAGS) -o producer producer.c

consumer: consumer.c
	$(CC) $(CFLAGS) -o consumer consumer.c

run_producer: producer
	./producer

clean:
	rm -f producer consumer
