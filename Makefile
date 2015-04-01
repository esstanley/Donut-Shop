donutshop: donuts.h producer.o consumer.o sigwaiter.o main.o
	cc -o pc_threads main.o consumer.o producer.o sigwaiter.o -lpthread
main.o:	donuts.h main.c
	cc -c main.c
consumer.o: donuts.h consumer.c
	cc -c consumer.c
producer.o: donuts.h producer.c
	cc -c producer.c
sigwaiter.o:  donuts.h sigwaiter.c
	cc -c sigwaiter.c
clean:
	rm *.o pc_threads

