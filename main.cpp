#include <stdio.h>
#include "var.h"

extern void TESTscan();
extern int TESTparse();
extern int TESTmachine();

extern err all[50];

FILE *fin, *fout;
char scanIn[300], scanOut[300];

int main()
{
	int es = 0;
	TESTscan();
	es = TESTparse();
	if (es > 0)
		printf("�﷨�����д���,�����������������\n");
	else
		es = TESTmachine();
	if (es > 0)
		printf("��������г��ִ���\n");
	return 0;
}