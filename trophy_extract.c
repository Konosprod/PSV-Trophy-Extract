#include <stdio.h>
#include <stdlib.h>

typedef struct Token {
	char filename[0x24];
	int offset;
	int size;
	char unknown[0x10];
}Token;

typedef struct Header {
	int mime;
	int version;
	int filesize;
	int nbfile;
	int offset;
	char unknown[0x24];
	Token* tok;
}Header;


void get_header(Header* head, FILE* in);
void get_tokens(Header* head, FILE* in);
void dump_files(Header* head, FILE* in);

int swap32(int num);

int main(int argc, char* argv[])
{
	FILE* in = fopen("TROPHY.TRP", "rb");

	Header head;

	get_header(&head, in);
	get_tokens(&head, in);
	dump_files(&head, in);

	free(head.tok);

	fclose(in);

}

void get_header(Header* head, FILE* in)
{
	fread(&head->mime, sizeof(char)*4,1, in);

	printf("Mime type :\t\t%.8X\n", head->mime);

	fread(&head->version, sizeof(char)*4, 1, in);

	printf("Version :\t\t%.8X\n", head->version);

	fseek(in, 0x04, SEEK_CUR);

	fread(&head->filesize, sizeof(char), 4, in);

	head->filesize = swap32(head->filesize);

	fread(&head->nbfile, sizeof(char), 4, in);

	head->nbfile = swap32(head->nbfile);

	printf("Number of file :\t%.8X\n", head->nbfile);

	fread(&head->offset, sizeof(char), 4, in);

	head->offset = swap32(head->offset);

	fread(&head->unknown, sizeof(char), 0x24, in);

	head->tok = calloc(head->nbfile, sizeof(Token));
}

void get_tokens(Header* head, FILE* in)
{
	fseek(in, head->offset, SEEK_SET);

	for(int i = 0; i < head->nbfile; i++)
	{
		fread(&head->tok[i].filename, sizeof(char), 0x24, in);
		fread(&head->tok[i].offset, sizeof(char), 4, in);

		head->tok[i].offset = swap32(head->tok[i].offset);

		fseek(in, 0x04, SEEK_CUR);

		fread(&head->tok[i].size, sizeof(char), 4, in);

		head->tok[i].size = swap32(head->tok[i].size);

		fread(&head->tok[i].unknown, sizeof(char), 0x10, in);
	}
}

void dump_files(Header* head, FILE* in)
{
	char b = 0;

	for(int i = 0; i < head->nbfile; i++)
	{
		FILE* out = fopen(head->tok[i].filename, "wb");

		printf("Dumping %s...", head->tok[i].filename);
		fflush(stdout);

		fseek(in, head->tok[i].offset, SEEK_SET);

		for(int j = 0; j < head->tok[i].size; j++)
		{
			fread(&b, sizeof(char), 1, in);
			fwrite(&b, sizeof(char), 1, out);
		}

		printf("...done !\n");

		fclose(out);
	}
}

int swap32(int num)
{
	int swapped = ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
	return swapped;
}
