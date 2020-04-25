#include <dirent.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef struct dbEntry {
	unsigned int id;
	char name[30];
	char type[10];
	unsigned int parent_id;
	unsigned char md5[MD5_DIGEST_LENGTH];
} dbEntry;

void md5digest(FILE* file, unsigned char* dest) {
	MD5_CTX md5handler;
	char data;
	unsigned char md5digest[MD5_DIGEST_LENGTH];

	MD5_Init(&md5handler);
	while ((fread(&data, 1, 1, file) != 0)) {
		MD5_Update(&md5handler, &data, 1);
	}
	MD5_Final(md5digest, &md5handler);
	strcpy((char*)dest, (char*)md5digest);
}

int md5match(unsigned char* md5_1, unsigned char* md5_2) {
	if (!md5_1 || !md5_2) {
		return 1;
	}

	int i;
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		if (md5_1[i] != md5_2[i]) {
			return 1;
		}
	}

	return 0;
}

int main(int argc, const char** argv) {
	if (strcmp(argv[1], "-s") == 0 && strcmp(argv[2], "-f") == 0 &&
	    argc == 5) {
		FILE* database = fopen(argv[3], "wb");
		DIR* dir = opendir(argv[4]);
		struct dirent* dirent = readdir(dir);
		if (dirent == NULL) {
			fprintf(stderr,
				"Directory %s doesn't exist or can't be "
				"opened. Check permissions\n",
				argv[4]);
			exit(-1);
		}
		unsigned int id = 0;
		dbEntry entry;
		while (dirent != NULL) {
			if (dirent->d_type == 8) {
				entry.id = id;
				strcpy(entry.name, dirent->d_name);
				strcpy(entry.type, "file");
				entry.parent_id = 0;
				const size_t len =
				    strlen(argv[4]) + strlen(dirent->d_name);
				char* fpath = malloc(sizeof(char) * len);
				strcpy(fpath, argv[4]);
				strcat(fpath, dirent->d_name);
				printf("%s\n", fpath);
				FILE* file = fopen(fpath, "rb");
				if (file != NULL) {
					md5digest(file, entry.md5);
					fclose(file);
					fwrite(&entry,
					       sizeof(struct dbEntry) +
						   sizeof(unsigned char) *
						       MD5_DIGEST_LENGTH +
						   54,
					       1, database);
				} else {
					printf("Error opening file %s\n",
					       entry.name);
					fpath = "";
				}
				id++;
			}
			dirent = readdir(dir);
		}
		fclose(database);

	} else if (strcmp(argv[1], "-c") == 0 && strcmp(argv[2], "-f") == 0 &&
		   argc == 5) {
		FILE* database = fopen(argv[3], "rb");
		if (database == NULL) {
			fprintf(stderr, "File %s doesn't exist\n", argv[3]);
			exit(-1);
		}
		DIR* dir = opendir(argv[4]);
		if (dir == NULL) {
			fprintf(stderr, "Directory %s doesn't exist\n",
				argv[4]);
			exit(-1);
		}
		struct dirent* dirent = readdir(dir);
		if (dirent == NULL) {
			fprintf(stderr, "Directory %s doesn't exist\n",
				argv[4]);
			exit(-1);
		}
		dbEntry* entry = malloc(sizeof(dbEntry));
		while (dirent != NULL) {
			if (dirent->d_type == 8) {
				fread(entry,
				      sizeof(struct dbEntry) +
					  sizeof(unsigned char) *
					      MD5_DIGEST_LENGTH +
					  54,
				      1, database);
				if (strcmp(dirent->d_name, entry->name) == 0) {
					unsigned char md5[MD5_DIGEST_LENGTH];
					char* name = entry->name;
					const size_t len =
					    strlen(argv[4]) + strlen(name);
					char* fpath =
					    malloc(sizeof(char) * len);
					strcpy(fpath, argv[4]);
					strcat(fpath, name);
					printf("%s\n", fpath);
					FILE* file = fopen(fpath, "rb");
					if (file != NULL) {
						md5digest(file, md5);
						if (md5match(md5, entry->md5) ==
						    0) {
							printf(
							    ANSI_COLOR_GREEN
							    "File %s hadn't "
							    "changed"
							    "\n" ANSI_COLOR_RESET,
							    entry->name);
						} else {
							printf(
							    ANSI_COLOR_YELLOW
							    "FILE %s "
							    "CHANGED"
							    "\n" ANSI_COLOR_RESET,
							    entry->name);
						}
					} else
						printf(
						    ANSI_COLOR_RED
						    "Error opening file "
						    "%s/file is "
						    "empty\n" ANSI_COLOR_RESET,
						    entry->name);
					printf("\n");
					fpath = "";
				}
			}
			dirent = readdir(dir);
		}
	}
	return 0;
}

