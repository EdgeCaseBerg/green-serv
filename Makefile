#Define some basics:
#Compiler is cc because why not.
CC=cc

#Make sure to compile with: $(mysql_config --cflags) and link to: $(mysql_config --libs)
.PHONY: clean tests

#a few handy defines to make our compilation lines not so long:
gflags = -I./headers -std=gnu99 -pedantic -Wall -Wextra -Werror -g
mysqlflags = -I/usr/include/mysql -DBIG_JOINS=1 -fno-strict-aliasing
mysqllibs  = -L/usr/lib/x86_64-linux-gnu -lmysqlclient_r -lpthread -lz -lm -lrt -ldl 
valgrind = valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes
unittests = test-decimal test-comment test-scope test-marker test-report test-heatmap test-heartbeat test-router test-network
unittestobj = obj/comment.o  obj/db.o  obj/decimal.o  obj/heatmap.o  obj/json.o obj/marker.o  obj/report.o  obj/scope.o  obj/sha256.o
controllertests = test-hb-controller

all: a.out

a.out: gs.o
	$(CC)  obj/*o  -o a.out $(mysqllibs) -g -lcrypto -lpthread

gs.o: green-serv.c json.o db.o network.o
	$(CC) $(gflags) -c green-serv.c -o obj/gs.o  	

db.o: src/database/db.c scope.o comment.o marker.o heatmap.o report.o
	$(CC) $(mysqlflags) $(gflags) -c src/database/db.c -o obj/db.o  	

json.o: src/helpers/json.c comment.o scope.o marker.o heatmap.o report.o decimal.o
	$(CC) $(gflags) -c src/helpers/json.c -o obj/json.o  	

scope.o: src/models/scope.c
	$(CC) $(gflags) -c src/models/scope.c -o obj/scope.o  	

comment.o: src/models/comment.c
	$(CC) $(gflags) -c src/models/comment.c -o obj/comment.o  	

marker.o: src/models/marker.c decimal.o
	$(CC) $(gflags) -c src/models/marker.c -o obj/marker.o  		

heatmap.o: src/models/heatmap.c decimal.o
	$(CC) $(gflags) -c src/models/heatmap.c -o obj/heatmap.o  			

report.o: src/models/report.c sha256.o
	$(CC) $(gflags) -c src/models/report.c -o obj/report.o  			

sha256.o: src/helpers/sha256.c
	$(CC) $(gflags) -c src/helpers/sha256.c -o obj/sha256.o

decimal.o: src/helpers/decimal.c
	$(CC) $(gflags) -c src/helpers/decimal.c -o obj/decimal.o

heartbeatC.o: src/controllers/heartbeat.c
	$(CC) $(gflags) -c src/controllers/heartbeat.c -o obj/heartbeatC.o

commentC.o: src/controllers/comments.c strmap.o comment.o router.o
	$(CC) $(gflags) -c src/controllers/comments.c -o obj/commentC.o

markerC.o: src/controllers/markers.c router.o strmap.o marker.o
	$(CC) $(gflags) -c src/controllers/markers.c -o obj/markerC.o

heatmapC.o: src/controllers/heatmaps.c router.o strmap.o heatmap.o
	$(CC) $(gflags) -c src/controllers/heatmaps.c -o obj/heatmapC.o

reportsC.o: src/controllers/reports.c router.o strmap.o report.o sha256.o
	$(CC) $(gflags) -c src/controllers/reports.c -o obj/reportsC.o

network.o: src/network/net.c router.o heartbeatC.o commentC.o markerC.o heatmapC.o reportsC.o
	$(CC) $(gflags) -c src/network/net.c -o obj/network.o 

router.o: src/network/router.c strmap.o
	$(CC) $(gflags) -c src/network/router.c -o obj/router.o

strmap.o: src/helpers/strmap.c
	$(CC) $(gflags) -c src/helpers/strmap.c -o obj/strmap.o

clean:
	rm obj/*.o *.out

tests: units controllers	

#Unit Tests
units: $(unittests)
	$(valgrind) tests/bin/scope.out
	$(valgrind) tests/bin/comment.out
	$(valgrind) tests/bin/decimal.out
	$(valgrind) tests/bin/marker.out
	$(valgrind) tests/bin/report.out
	$(valgrind) tests/bin/heatmap.out
	$(valgrind) tests/bin/heartbeat.out
	$(valgrind) tests/bin/router.out
	$(valgrind) tests/bin/network.out

test-decimal: tests/unit/decimal-test.c decimal.o
	$(CC) $(gflags) tests/unit/decimal-test.c obj/decimal.o -o tests/bin/decimal.out -lm -rdynamic

test-comment: tests/unit/comment-test.c comment.o json.o db.o 
	$(CC) $(mysqlflags) $(gflags) tests/unit/comment-test.c $(unittestobj) -o tests/bin/comment.out $(mysqllibs) -lcrypto

test-scope: tests/unit/scope-test.c scope.o json.o db.o
	$(CC) $(mysqlflags) $(gflags) tests/unit/scope-test.c $(unittestobj) -o tests/bin/scope.out $(mysqllibs) -lcrypto		

test-marker: tests/unit/marker-test.c marker.o comment.o json.o db.o
	$(CC) $(mysqlflags) $(gflags) tests/unit/marker-test.c $(unittestobj) -o tests/bin/marker.out $(mysqllibs) -lcrypto		

test-report: tests/unit/report-test.c report.o json.o db.o
	$(CC) $(mysqlflags) $(gflags) tests/unit/report-test.c $(unittestobj) -o tests/bin/report.out $(mysqllibs) -lcrypto

test-heatmap: tests/unit/heatmap-test.c heatmap.o json.o db.o decimal.o
	$(CC) $(mysqlflags) $(gflags) tests/unit/heatmap-test.c $(unittestobj) -o tests/bin/heatmap.out $(mysqllibs) -lcrypto	

test-heartbeat: tests/unit/heartbeat-test.c json.o decimal.o
	$(CC) $(gflags) tests/unit/heartbeat-test.c obj/json.o obj/decimal.o -o tests/bin/heartbeat.out 

test-router: tests/unit/router-test.c router.o strmap.o
	$(CC) $(gflags) tests/unit/router-test.c obj/router.o obj/strmap.o -o tests/bin/router.out

test-network: tests/unit/network-test.c router.o  strmap.o network.o commentC.o markerC.o heartbeatC.o db.o comment.o report.o marker.o scope.o heatmap.o
	$(CC) $(mysqlflags) $(gflags) tests/unit/network-test.c $(unittestobj) obj/router.o obj/network.o obj/strmap.o obj/commentC.o obj/markerC.o  obj/heartbeatC.o -o tests/bin/network.out -lpthread $(mysqllibs) -lcrypto

#Controller Tests

controllers: $(controllertests)
	$(valgrind) tests/bin/heartbeatC.out

test-hb-controller: tests/controllers/heartbeat-test.c heartbeatC.o json.o
	$(CC) $(gflags) tests/controllers/heartbeat-test.c obj/heartbeatC.o obj/json.o obj/decimal.o -o tests/bin/heartbeatC.out


#Integration Tests with network 
