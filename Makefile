CFLAGS = -fopenmp -lpthread -mavx2 -Wall -DGETTIMEOFDAY -std=c99 -O2

all : opt1.out opt2.out opt3.out opt4.out opt5.out \
	  opt6.out opt7.out opt8.out opt9.out opt10.out \
	  opt11.out opt12.out opt13.out opt14.out

opt1.out : opt-1.c
	gcc -o opt1.out opt-1.c common.h $(CFLAGS)

opt2.out : opt-2.c
	gcc -o opt2.out opt-2.c common.h $(CFLAGS)

opt3.out : opt-3.c
	gcc -o opt3.out opt-3.c common.h $(CFLAGS)

opt4.out : opt-4.c
	gcc -o opt4.out opt-4.c common.h $(CFLAGS)

opt5.out : opt-5.c
	gcc -o opt5.out opt-5.c common.h $(CFLAGS)

opt6.out : opt-6.c
	gcc -o opt6.out opt-6.c common.h $(CFLAGS)

opt7.out : opt-7.c
	gcc -o opt7.out opt-7.c common.h $(CFLAGS)

opt8.out : opt-8.c
	gcc -o opt8.out opt-8.c common.h $(CFLAGS)

opt9.out : opt-9.c
	gcc -o opt9.out opt-9.c common.h $(CFLAGS)

opt10.out : opt-10.c
	gcc -o opt10.out opt-10.c common.h $(CFLAGS)

opt11.out : opt-11.c
	gcc -o opt11.out opt-11.c common.h $(CFLAGS)

opt12.out : opt-12.c
	gcc -o opt12.out opt-12.c common.h $(CFLAGS)

opt13.out : opt-13.c
	gcc -o opt13.out opt-13.c common.h $(CFLAGS)

opt14.out : opt-14.c
	gcc -o opt14.out opt-14.c common.h $(CFLAGS)

clean:
	rm -rf *.out