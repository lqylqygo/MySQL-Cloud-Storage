# 目标：生成最终的可执行文件 servertestconf
servertestconf: config.o main.o hashtable.o
	gcc config.o main.o hashtable.o -o servertestconf -g -Wall -lpthread

# config.o 依赖于 config.c 和 config.h
config.o: config.c config.h
	gcc -c config.c -o config.o -g -Wall

# hashtable.o 依赖于 hashtable.c 和 hashtable.h
hashtable.o: hashtable.c hashtable.h
	gcc -c hashtable.c -o hashtable.o -g -Wall

# main.o 依赖于 main.c 以及用到的头文件 (这样改了头文件，main也会重编)
main.o: main.c config.h hashtable.h
	gcc -c main.c -o main.o -g -Wall

# 伪目标，用于清理编译生成的文件
clean:
	rm -f *.o servertestconf
