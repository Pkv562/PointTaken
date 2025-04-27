SERVER = server
CLIENT = client
CC = gcc
CFLAGS = -Wall -Wextra -g

all: $(SERVER) $(CLIENT)

$(SERVER): server.c common.h point_taken.c
	$(CC) $(CFLAGS) -o $(SERVER) server.c

$(CLIENT): client.c common.h point_taken.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c

clean:
	rm -f $(SERVER) $(CLIENT)