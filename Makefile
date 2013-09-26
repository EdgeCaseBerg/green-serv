#Make sure to compile with: $(mysql_config --cflags) and link to: $(mysql_config --libs)

a.out: gs.o
	cc  obj/db.o obj/gs.o obj/json.o obj/scope.o obj/comment.o -o a.out -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl -g  	

gs.o: green-serv.c json.o db.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c green-serv.c -o obj/gs.o  	

db.o: src/database/db.c scope.o comment.o
	cc -I./headers -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/database/db.c -o obj/db.o  	

json.o: src/helpers/json.c comment.o scope.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/helpers/json.c -o obj/json.o  	

scope.o: src/models/scope.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/scope.c -o obj/scope.o  	

comment.o: src/models/comment.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/comment.c -o obj/comment.o  	

clean:
	rm obj/*.o *.out
