All : Asteroids Move_ship Main Main 


Asteroids : asteroids.c asteroids.h
	-gcc -Wall -c -o asteroids.o asteroids.c

Move_ship : move_ship.c asteroids.h
	-gcc -Wall -c -o move_ship.o move_ship.c

Main : asteroids.c asteroids.h move_ship.c move_ship.o
	-gcc -Wall -o main asteroids.o move_ship.o -lncurses 