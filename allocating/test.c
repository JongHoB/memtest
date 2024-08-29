#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <err.h>
#include <fcntl.h>

#define CHUNK (2<<20)


int main(int argc, char *argv[])
{
	long long btotal = 0;
	char *end = NULL;
	int thp=0;

	if(argc <3){
		perror("error");
		exit(-1);
	}


	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);

	if(argc >=2)
	{
		
		btotal = strtoull(argv[1], &end ,10);

		switch (*end)
		{
			case 'g':
			case 'G':
				btotal *= (1<<10);
			case 'm':
			case 'M':
				btotal *= (1<<10);
			case 'k':
			case 'K':
				btotal *= (1<<10);
				break;
			default :
				exit(-1);
				break;
		}
	}
	
	if(argc >=3)
		thp=atoi(argv[2]);

	long long numchunk = btotal / CHUNK;
	printf("allocate %lld chunks\n", numchunk);

	void **data = mmap(0 , sizeof(void*) *numchunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf(" allocatd **data size %llu\n", sizeof(void*)*numchunk);
		
retry: 
	printf("allocate memory\n");
	for(int i = 0; i <numchunk;i++)
	{
		data[i] = mmap(NULL, CHUNK, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);	

		if(thp){
			if(madvise(data[i], CHUNK, MADV_HUGEPAGE) <0)
			{
				perror("error");
				exit(-1);
			}
		}
		else{
			if(madvise(data[i], CHUNK, MADV_NOHUGEPAGE) <0)
			{
				perror("error");
				exit(-1);
			}
		}

		memset(data[i], 1, CHUNK);

	}
	printf("done\n");

	printf("retry? y/n :\n");
	char tmp[2];
	scanf("%s",tmp);



	for (int i =0; i< numchunk;i++)
	{
		munmap(data[i], CHUNK);
	}

	if(strcmp(tmp,"y")!=0)
	{
		printf("done\n");
		return 0;
	}
			
	goto retry;
}

