#Make sure to compile with: $(mysql_config --cflags) and link to: $(mysql_config --libs)

a.out: db.o
	cc  obj/db.o -o a.out -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl

db.o: src/db.c
	cc -I./headers -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/db.c -o obj/db.o

clean:
	rm obj/*.o *.out
