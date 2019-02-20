#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//cache 
//memory array 

typedef struct frame{
	int tag;
	int validbit;
} frame;

int log2(int n){
	int r = 0;
	while (n>>=1) r++;
	return r;
}
int getLowerN(int input, int N){
	int ones = ((1<<N)-1);
	int output = input & ones;
	return output;
}
int deleteLowerN(int input, int N){
	int output = input>>N;
	return output;
}
int main (int argc, char* argv[]){

	//process command line inputs 
	FILE *file = fopen(argv[1], "r");
	unsigned int cachesize = atoi(argv[2]);
	unsigned int associativity = atoi(argv[3]);
	unsigned int blocksize = atoi(argv[4]);

	unsigned int num_sets = (cachesize<<10)/blocksize/associativity;
	unsigned int num_frames = cachesize/blocksize;	//= num_sets * associativity

	//initialize empty cache 
	frame* cache = (struct frame*) malloc (num_frames * sizeof(frame));
	for (int i = 0; i < num_frames; i++){
		cache[i].tag = 0;
		cache[i].validbit = 0;
	}

	//initialize 16MB memory 
	char* memory = (char*) malloc((1<<24) * sizeof(char));
	for (int i = 0; i < (1<<24); i++){
		memory[i] = 0;
	}

	unsigned int address;
	unsigned int access_bytes;
	char storeval[blocksize];	//what's the most efficient length?
	char* storevalpointer = storeval;

	unsigned int tag;
	unsigned int set_index;
	unsigned int block_ofs; 

	unsigned int Ntag = log2(associativity);
	unsigned int Nset_index = log2(num_sets);
	unsigned int Nblock_ofs = log2(blocksize);

	bool store;

	while (1){
		//determine load or store
		char *command; 
		if (fscanf(file, "%s", command) == EOF) return EXIT_SUCCESS;
		store = 0; 										//reset to false 
		if (strcmp(command, "store") == 0) store = 1; 	//set to true if is store 

		fscanf (file, "%x", &address);
		fscanf (file, "%u", &access_bytes);
		if (store){
			fscanf(file, "%s", storeval);
		}

		block_ofs = getLowerN(address, Nblock_ofs);
		set_index = getLowerN(deleteLowerN(address, Nblock_ofs), Nset_index);	//why delete block ofs?
		tag = deleteLowerN(address, Nblock_ofs + Nset_index);

		printf("%s 0x%x ", command, address);

		bool hit = 0;

		for (int i = 0; i < associativity; i++){
			frame this_frame = cache[associativity * set_index + i];
			unsigned int hitframeindex;
			
			//if hit 
			if ((this_frame.tag == tag) && (this_frame.validbit == 1)){
				hit = 1;
				hitframeindex = i;

				//store hit 
				if (store){
					printf("hit\n");
					storevalpointer = storeval;
					//store data in memory 
					for (int j = 0; j < access_bytes; j++){
						sscanf(storevalpointer, "%02hhx", &memory[address + j]);
						storevalpointer += 2; //why 2? 
					}
					//store data in cache 
					frame temp_frame = this_frame;
					for (int j = hitframeindex; j > 0; j--){
						cache[associativity * set_index + j] = cache[associativity * set_index + j - 1];
					}
					cache[associativity * set_index] = temp_frame;
					break; //next command 
				}

				//load hit
				else{
					frame temp_frame = this_frame;
					for (int j = hitframeindex; j > 0; j--){
						cache[associativity * set_index + j] = cache[associativity * set_index + j - 1];
					}
					cache[associativity * set_index] = temp_frame;
					printf("hit ");
					for (int j = 0; j < access_bytes; j++){
						printf("%02hhx", memory[address + j]);
					}
					printf("\n");
					break; //next command 
				}
			}
		}

		//if miss
		if (hit == 0){

			//store miss
			if (store){
				printf("miss\n");
				storevalpointer = storeval;
				for (int j = 0; j < access_bytes; j++){
					sscanf(storevalpointer, "%02hhx", &memory[address + j]);
					storevalpointer += 2;
				}
			}

			//load miss
			else{
				//move every frame back by 1 
				for (int i = associativity - 1; i>0; i--){
					cache[associativity * set_index + i] = cache[associativity * set_index + i - 1];
				}
				//put new block in first frame
				cache[associativity * set_index].validbit = 1;
				cache[associativity * set_index].tag = tag;
				printf("miss ");
				for (int j = 0; j < access_bytes; j++){
					printf ("%02hhx", memory[address + j]);
				}
				printf("\n");
			}
		}
	}
}
















