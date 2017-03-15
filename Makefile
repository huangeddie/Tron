all : tron_server.out tron_client.out

tron_server.out : tron_server.c move_manager.c terminal_graphics/color_graphics.o networking/server.o constants.h
	gcc -std=gnu99 tron_server.c terminal_graphics/color_graphics.o networking/server.o -o tron_server.out -lncurses -pthread -Wall -O0 -g -lm

tron_client.out : tron_client.c terminal_graphics/color_graphics.o networking/client.o constants.h
	gcc -std=gnu99 tron_client.c terminal_graphics/color_graphics.o networking/client.o -o tron_client.out -lncurses -pthread -Wall -O0 -g -lm

terminal_graphics/color_graphics.o : terminal_graphics/color_graphics.c terminal_graphics/color_graphics.h terminal_graphics/queue.c
	gcc -std=gnu99 -c terminal_graphics/color_graphics.c -lncurses -pthread -Wall -O0 -g -o terminal_graphics/color_graphics.o

networking/server.o : networking/server.h networking/server.c constants.h networking/network_helpers.c
	gcc -std=gnu99 -g -c -o networking/server.o -std=gnu99 networking/server.c -lpthread -Wall

networking/client.o : networking/client.h networking/client.c constants.h networking/network_helpers.c
	gcc -std=gnu99 -g -c -o networking/client.o -std=gnu99 networking/client.c -lpthread -Wall

ser_test_client.o : move_manager.c ser_test_client.c networking/client.o
	gcc -std=gnu99 -g -Wall ser_test_client.c networking/client.o -o ser_test_client.o -lm

ser_test_server.o : move_manager.c ser_test_server.c networking/server.o
	gcc -std=gnu99 -g -Wall ser_test_server.c networking/server.o -o ser_test_server.o -lm

test.o : move_manager.c test.c
	gcc -std=gnu99 -g -Wall test.c -o test.o -lm

clean :
	find networking/ -name "*.o" -type f -delete
	find terminal_graphics/ -name "*.o" -type f -delete
	find . -name "*.o" -type f -delete
	find . -name "*.out" -type f -delete
