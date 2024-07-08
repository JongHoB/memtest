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

void usr_handler(int signal)
{
		printf("catch signal\n");
			exit(-1);
}
int main(int argc, char *argv[])
{
	long long btotal = 0;
	char *end = NULL;

	printf("%s\n", argv[1]);

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
	long long numchunk = btotal / CHUNK;
	printf("allocate %lld chunks\n", numchunk);

	void **data = mmap(0 , sizeof(void*) *numchunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf(" allocatd **data size %llu\n", sizeof(void*)*numchunk);
	
	struct sigaction sa;
	sigset_t set;
	
	sa.sa_flags = 0;
	sa.sa_handler = usr_handler;

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		errx(1, "sigaction");

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	
retry: 
	printf("allocate memory\n");
	for(int i = 0; i <numchunk;i++)
	{
		data[i] = mmap(NULL, CHUNK, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);	

		if(madvise(data[i], CHUNK, MADV_NOHUGEPAGE) <0)
		{
			perror("error");
			exit(-1);
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

