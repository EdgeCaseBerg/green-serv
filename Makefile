#Make sure to compile with: $(mysql_config --cflags) and link to: $(mysql_config --libs)

a.out: gs.o
	cc  obj/*o  -o a.out -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl -g -lcrypto

gs.o: green-serv.c json.o db.o 
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c green-serv.c -o obj/gs.o  	

db.o: src/database/db.c scope.o comment.o marker.o heatmap.o report.o
	cc -I./headers -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/database/db.c -o obj/db.o  	

json.o: src/helpers/json.c comment.o scope.o marker.o heatmap.o report.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/helpers/json.c -o obj/json.o  	

scope.o: src/models/scope.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/scope.c -o obj/scope.o  	

comment.o: src/models/comment.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/comment.c -o obj/comment.o  	

marker.o: src/models/marker.c decimal.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/marker.c -o obj/marker.o  		

heatmap.o: src/models/heatmap.c decimal.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/heatmap.c -o obj/heatmap.o  			

report.o: src/models/report.c sha256.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/models/report.c -o obj/report.o  			

sha256.o: src/helpers/sha256.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/helpers/sha256.c -o obj/sha256.o

decimal.o: src/helpers/decimal.c
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g -c src/helpers/decimal.c -o obj/decimal.o

clean:
	rm obj/*.o *.out

tests: test-decimal test-comment
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes tests/bin/*.out

test-decimal: tests/decimal-test.c decimal.o
	cc -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g tests/decimal-test.c obj/decimal.o -o tests/bin/decimal.out -lm -rdynamic

test-comment: tests/comment-test.c comment.o json.o db.o marker.o report.o heatmap.o
	cc -I./headers -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing -std=gnu99 -pedantic -Wall -Wextra -Werror -g tests/comment-test.c obj/*.o -o tests/bin/comment.out -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl -g -lcrypto