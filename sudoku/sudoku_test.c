#include <stdio.h>
#include "sudoku.c"


int main(int argc, char** argv)
{
	// char str[LENGHT * LENGHT + 1] = "090005000607000010020400908000070800205090104008050000709003060050000703000100092";
	char str[LENGHT * LENGHT + 1] = "090005000607000010020400908000070800205090104008050000709003060050000703000100095";
	if (0 == sudoku(str))
	{
		printf("%s\n", str);
	}
	else
	{
		printf("no answer\n");
	}

	return 0;
}
