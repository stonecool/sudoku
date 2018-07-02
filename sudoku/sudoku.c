#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGHT 9

#define true 1
#define false 0

void print(int *sudoku/*, int *lFlag, int *cFlag, int * bFlag*/)
{
	int i = 0;
	for (i = 0; i < LENGHT * LENGHT; ++i)
	{
		printf("%d ", sudoku[i]);

		if (i % LENGHT == LENGHT - 1)
		{
			printf("\n");
		}
	}
/*
	printf("line flag: \n");
	for (i = 0 ;i < LENGHT; ++i)
	{
		for (j = 0 ; j < LENGHT; ++j)
		{
			if (lFlag[i] >> j & 1)
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}
	
	printf("column flag: \n");
	for (i = 0 ;i < LENGHT; ++i)
	{
		for (j = 0 ; j < LENGHT; ++j)
		{
			if (cFlag[i] >> j & 1)
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}
	
	printf("block flag: \n");
	for (i = 0 ;i < LENGHT; ++i)
	{
		for (j = 0 ; j < LENGHT; ++j)
		{
			if (bFlag[i] >> j & 1)
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}
	*/
}


int init(int *sudoku, int *lFlag, int *cFlag, int *bFlag)
{
	int i = 0, line = 0, column = 0, block = 0, num = 0;
	for (i = 0;i < LENGHT * LENGHT; ++i)
	{
		if (0 == sudoku[i])
			continue;

		line = i / LENGHT;
		column = i % LENGHT;
		num = sudoku[i] - 1;

		if (lFlag[line] >> num & 1)
		{
			perror("init line error");
			return 1;
		}
		lFlag[line] |= 1 << num;

		if (cFlag[column] >> num & 1)
		{
			perror("init column error");
			return 1;
		}
		cFlag[column] |= 1 << num;

		block = (line / 3) * 3 + column / 3;
		if (bFlag[block] >> num & 1)
		{
			perror("init block error");
			return 1;
		}

		bFlag[block] |= 1 << num;
	}

	return 0;
}


int resolve(int *sudoku, int *lFlag, int *cFlag, int *bFlag, int index)
{
	if (index == LENGHT * LENGHT)
		return true;

	if (sudoku[index] != 0)
		return resolve(sudoku, lFlag, cFlag, bFlag, index + 1);

	int line = index / LENGHT, column = index % LENGHT, num = 0, block = 0;

	block = (line / 3) * 3 + column / 3;
	for (num = 0; num < LENGHT; ++num)
	{
		if ((lFlag[line] >> num & 1) == 0 &&
			(cFlag[column] >> num & 1) == 0 &&
			(bFlag[block] >> num & 1) == 0)
		{
			sudoku[index] = num + 1;
			lFlag[line] |= 1 << num;
			cFlag[column] |= 1 << num;
			bFlag[block] |= 1 << num;
				
			if (! resolve(sudoku, lFlag, cFlag, bFlag, index + 1))
			{
				sudoku[index] = 0;
				lFlag[line] &= ~(1 << num);
				cFlag[column] &= ~(1 << num);
				bFlag[block] &= ~(1 << num);
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

void serialize(int *sudoku, char* input)
{
	int i = 0;

	for (i = 0;i < LENGHT * LENGHT; ++i)
	{
		input[i] = '0' + sudoku[i];		
	}
	input[i] = '\n';
}


void unserialize(char *input, int *sudoku)
{
	int i = 0;

	for (i = 0;i < LENGHT * LENGHT; ++i)
	{
		if ('0' <= input[i] && input[i] <= '9')
		{
			sudoku[i] = 0 + input[i] - '0';	
		}
		else
		{
			printf("u: %d c: %d\n", i, input[i]);
			perror("unserialize error");
			exit(1);
		}
	}
}


void sudoku(char *input)
{
	/*
	int sudoku[LENGHT * LENGHT] = {
		0, 9, 0, 0, 0, 5, 0, 0, 0,
		6, 0, 7, 0, 0, 0, 0, 1, 0,
		0, 2, 0, 4, 0, 0, 9, 0, 8,
		0, 0, 0, 0, 7, 0, 8, 0, 0,
		2, 0, 5, 0, 9, 0, 1, 0, 4,
		0, 0, 8, 0, 5, 0, 0, 0, 0,
		7, 0, 9, 0, 0, 3, 0, 6, 0,
		0, 5, 0, 0, 0, 0, 7, 0, 3,
		0, 0, 0, 1, 0, 0, 0, 9, 0
	};
	*/
	int sudoku[LENGHT * LENGHT] = {0};
	int lFlag[LENGHT] = {0}; 
	int cFlag[LENGHT] = {0}; 
	int bFlag[LENGHT] = {0}; 

	unserialize(input, sudoku);
	if (0 != init(sudoku, lFlag, cFlag, bFlag))
	{
		memset(input, 0, LENGHT * LENGHT);	
		strncpy(input, "error", 5);
		return;
	}
	
	if (false == resolve(sudoku, lFlag, cFlag, bFlag, 0))
	{
		memset(input, 0, LENGHT * LENGHT);	
		strncpy(input, "error", 5);
	}
	else
	{
		serialize(sudoku, input);
	}
}

