#Make sure to compile with: $(mysql_config --cflags) and link to: $(mysql_config --libs)

a.out: gs.o
	cc  obj/db.o obj/gs.o obj/json.o obj/scope.o -o a.out -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl -g

gs.o: src/green-serv.c json.o db.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/green-serv.c -o obj/gs.o

db.o: src/db.c scope.o
	cc -I./headers -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/db.c -o obj/db.o

json.o: src/json.c 
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/json.c -o obj/json.o

scope.o: src/scope.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/scope.c -o obj/scope.o

clean:
	rm obj/*.o *.out
