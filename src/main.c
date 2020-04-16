#include <sys/stat.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct dbEntry {
	unsigned int id;
	char* name;
	char* type;
	unsigned int parent_id;
	unsigned char* md5;
} dbEntry;

void md5digest(FILE* file, unsigned char* dest) {
	MD5_CTX md5handler;
	char data[10];
	unsigned char md5digest[MD5_DIGEST_LENGTH];

	MD5_Init(&md5handler);
	while ((fread(data, 1, 1, file) != 0)) {
		MD5_Update(&md5handler, data, 1);
	}
	MD5_Final(md5digest, &md5handler);
	strcpy((char*)dest, (char*)md5digest);
}


int main(int argc, const char** argv) {
	
	if (strcmp(argv[1], "-s") == 0 && strcmp(argv[2], "-f") == 0 &&
	    argc == 5) {
		FILE* database = fopen(argv[3], "ab+");
		DIR* dir = opendir(argv[4]);
		struct dirent* dirent = readdir(dir);
		if (dirent == NULL){
			fprintf(stderr, "Directory %s doesn't exist\n", argv[4]);
			exit(-1);
		}
		unsigned int id = 0;
		dbEntry entry;
		entry.md5 = malloc(sizeof(char)*MD5_DIGEST_LENGTH);
		while (dirent != NULL) {
			if (dirent->d_type == 8) {
				entry.id = id;
				entry.name = dirent->d_name;
				entry.type = "file";
				entry.parent_id = 0;
				const size_t len = strlen(argv[4]) + strlen(entry.name);
				char* fpath = malloc(sizeof(char)*len);
				strcpy(fpath, argv[4]);
				strcat(fpath, entry.name);
				printf("%s\n", fpath);
				FILE* file = fopen(fpath, "rb");
				if (file != NULL){
					md5digest(file, entry.md5);
					fclose(file);
					fwrite(&entry, sizeof(struct dbEntry), 1, database);
				}
				else{
				      printf("Error opening file %s\n", entry.name);
				      fpath = "";
				}
				id++;
			}
			dirent = readdir(dir);
		}
		fclose(database);

	}
	else if (strcmp(argv[1], "-c") == 0 && strcmp(argv[2], "-f") == 0 && argc == 5){
		FILE* database = fopen(argv[3], "rb");
		if (database == NULL){
			fprintf(stderr, "File %s doesn't exist\n", argv[3]);
			exit(-1);
		}
		DIR* dir = opendir(argv[4]);
		struct dirent* dirent = readdir(dir);
		if (dirent == NULL){
			fprintf(stderr, "Directory %s doesn't exist\n", argv[4]);
			exit(-1);
		}
		dbEntry entry;
		entry.md5 = malloc(sizeof(char)*MD5_DIGEST_LENGTH);
		while (dirent != NULL){
			if (dirent->d_type == 8){
				fread(&entry, sizeof(struct dbEntry), 1, database);
				printf("%s\n", entry.name);
				if (dirent->d_name == entry.name){
					unsigned char md5[MD5_DIGEST_LENGTH];
								
					const size_t len = strlen(argv[4]) + strlen(entry.name);
					char* fpath = malloc(sizeof(char)*len);
					strcpy(fpath, argv[4]);
					strcat(fpath, entry.name);
					printf("%s\n", fpath);
					FILE* file = fopen(fpath, "rb");
					if (file != NULL){
						md5digest(file, md5);
						if (strcmp((char*)md5, (char*)entry.md5) == 0){
							printf("File %s hadn't changed\n", entry.name);
						}
						else printf("FILE %s CHANGED\n", entry.name);
					}
					fpath = "";
				}
				
			}
			dirent = readdir(dir);
		}
	}
	return 0;
}


















