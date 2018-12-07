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
		printf("语法分析有错误,不能载入虚拟机运行\n");
	else
		es = TESTmachine();
	if (es > 0)
		printf("虚拟机运行出现错误\n");
	return 0;
}