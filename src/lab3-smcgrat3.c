#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

//represents lines in cache
typedef struct Line{
    char valid;
    unsigned long long int tag;
    int LRUCounter;
} Line;

int m = 0;
int s = 0;
int e = 0;
int b = 0; //global commandline inputs
int set_num;
int block_size;//only used while debugging
int LRU_count = 0;//incremented by 1 every time a new address is being run from file
Line **cache;//double array of lines to similate a cache
int hits = 0;//number of hits during runtime, worth 1 cycle
int misses = 0;//number of misses during runtime, worth 100 cycles
int cycles;//how many cycles there have been
int evictions = 0;
static int hitTime = 1;
static int missTime = 100;

//frees cache memory that was malloc'd earlier line by line, and afterwords free's the top-level pointer
void freeMemory(){
    for (int i=0; i<set_num; i++){
        free(cache[i]);
    }
    free(cache);
}

void miss(void){
    misses++;
    cycles += missTime + hitTime;
}

void hit(void){
    hits++;
    cycles += hitTime;
}

//searches through cache for hits or misses
//uses the set index of the address to choose which line to searches
//while looking through line checks for equal tags and valid bits to decide wether it is hit or miss, while keeping track of LRU for evictions on misses
void run_cache(unsigned long long int address){
	unsigned long long int set_index_m =  (int)(pow(2, s) - 1);
	unsigned long long int set_index = (address >> b) & set_index_m;
	unsigned long long int addr_tag = address >> (s + b);
 
	Line *cLine = cache[set_index];
	int i, evict_index = 0;
	
	for(i = 0; i < e; i++) {
        if(cLine[i].tag == addr_tag && cLine[i].valid == '1') {//hit found
            cLine[i].LRUCounter = LRU_count;
            hit();
			printf("%x H\n", address);
            break;
        }
        if(cLine[evict_index].LRUCounter > cLine[i].LRUCounter) evict_index = i;//i is the location that LRU found
    }
    if(i == e) {//no hits found so must be a miss
        miss();
		printf("%x M\n", address);
        if(cLine[evict_index].valid == '0') {//there is nothing to evict since its empty so just place tag
			cLine[evict_index].valid = '1';
            cLine[evict_index].tag = addr_tag;
            cLine[evict_index].LRUCounter = LRU_count;
        }
        else {
            cLine[evict_index].valid = '1';
            cLine[evict_index].tag = addr_tag;
            cLine[evict_index].LRUCounter = LRU_count;
            evictions++;//a 'block' was evicted by LRU so increment eviction count
        }
    }
}

void readAddress(){

    FILE * pFile;
	pFile = fopen ("address.txt","r");
	
	unsigned long long int address;
	
    while(fscanf(pFile, "%x", &address) > 0){
		LRU_count++;//the higher the counter the more recently the block was accessed, a 0 has never been accesed
		run_cache(address);
    }
	printf("[result] hits: %d, misses: %d, miss rate: %d percent, total running time: %d cycles\n", hits, misses, ((misses*100)/(hits+misses)) , cycles);
    fclose(pFile);
	freeMemory();
}

//used to create an empty cache
void setUp(){

    cache = (Line **)malloc(sizeof(Line*) * set_num);//create cache of correct size
    for (int i = 0; i < set_num; i++){
        cache[i] = (Line*)malloc(sizeof(Line) * e);
        for (int j = 0; j < e; j++){
            cache[i][j].valid = '0';//initialize valid bit
			cache[i][j].tag = 0;//init tag
			cache[i][j].LRUCounter = 0;//init lru counter
        }
    }

    readAddress();
}

int main(int argc, char** argv){
	
	int opt;
	
    while ((opt = getopt(argc, argv, "m:s:e:b:")) != -1) {
        switch (opt) {
            case 'm': 
			   m = strtol(optarg, NULL, 10);
               break;

            case 's': 
			   s = strtol(optarg, NULL, 10);
               break;

            case 'e': 
			   e = strtol(optarg, NULL, 10);
               break;
			
			case 'b':
				b = strtol(optarg, NULL, 10);
				break;
            default:
                break;
        }
    }

	if (s == 0 || e == 0 || b == 0 || m == 0) {//assuming that e = 1 is a direct mapped cache and not e = 0.
        printf("[ERROR] %s: Missing required command line argument\n", argv[0]);
        return 1;
    }
	
	set_num = (unsigned int) pow(2, s);
    block_size = (unsigned int) pow(2, b);
	
    setUp();
	
    return 0;
}