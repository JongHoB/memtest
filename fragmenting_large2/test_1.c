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
#include <ctype.h>


#define CHUNK (2<<20)

unsigned int fragmentation_score_node1(){

	int nr_normal = 33554432; 

	int free_normal = 0;
		
	int suit_normal = 0;

	int node1_total = nr_normal;

	FILE *fp;
	char buffer[256];
	
	fp = fopen("/proc/buddyinfo","r");
	if(fp ==NULL)
	{
		perror("Error opening /proc/buddyinfo");
		exit(-1);
	}
	
	while(fgets(buffer, sizeof(buffer) , fp) !=NULL) {
		char node_info[32];
		int node_number=0;
		
		sscanf(buffer, "%s",node_info);
		if(strcmp(node_info,"Node") != 0)
			continue;
		sscanf(buffer,"Node %d,", &node_number);
		if(node_number !=1)
			continue;
		
		char *ptr = buffer;
		char zone_name[32];		

		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;

		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
		

		
		sscanf(ptr, "%*[^ ] %s", zone_name);

//		printf("Zone: %s\n", zone_name);
		
//		printf("%ld\n",strlen(zone_name));		
		
		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
		ptr+=strlen(zone_name);
		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
	
	
//		printf("%s",ptr);

		int block_count=0;
		
		ptr = buffer;
		int block_size=1;
		char bs[32];

		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
	
		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
	
		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
	
		while(*ptr && *ptr != ' ') ptr++;
		while(*ptr && *ptr == ' ') ptr++;
	
	
		while(sscanf(ptr,"%s", bs) == 1) {
//			printf("%d\n",atoi(bs));
			block_count = atoi(bs);
			
			
			if(strcmp(zone_name, "Normal") ==0){
				free_normal += block_size * block_count;

				if(block_size >= 512)
					suit_normal += block_size * block_count;
			}	
			
			block_size *= 2;		
			if(block_size > 1024)
				break;


			while(*ptr && *ptr != ' ') ptr++;
			while(*ptr && *ptr == ' ') ptr++;
		}
	}
	fclose(fp);
	
	unsigned int fragmentation_score_normal = ((free_normal - suit_normal)*100)/free_normal;

	long long frag_normal = nr_normal * fragmentation_score_normal;
	
	unsigned int fragmentation_score = (unsigned int)((frag_normal) / node1_total);
	
//	printf("%d\n",fragmentation_score);
	return fragmentation_score;		


}

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
	printf("allocate memory Node1\n");
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
	printf("fragmenting Node1\n");
	for(int i =1000 ; i< numchunk ;i++)
	{
		for(int j = (4<<10) ; j < CHUNK ; j += (4<<10)*2)
		{
			size_t free_size = 4<<10;
			
			if(madvise(data[i] + j, free_size, MADV_DONTNEED) < 0)
			{
				perror("error");
				exit(-1);
			}
//			usleep(10);
			if( i > numchunk/2) 
			{	int score = fragmentation_score_node1();
				if(score >=90)
				{	printf("frag score : %d\n",score);
					goto done;
				}
			}
		}
	}

done:
	sleep(8);
	printf("after frag score : %d\n", fragmentation_score_node1());
	
	char re[2];

	printf("retry? : y/n \n");
	scanf("%s",re);
	for (int i =0; i< numchunk;i++)
	{
		munmap(data[i], CHUNK);
	}
	if(strcmp(re,"y")==0)
		goto retry;
	else
		return 0;
}

