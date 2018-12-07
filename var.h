#pragma once
typedef struct err
{
	int lineNum;
	int es;
}err;


enum varKind { variable, array, globe };    //变量的三种类型（普通，数组，全局变量）


typedef struct varItem    //变量符号表
{
	char name[20];      //变量名
	char inFunc[20];     //变量所属的函数名
	enum varKind kind;
	int address;
	struct varItem* next;
}varItem;


typedef struct funItem     //函数符号表
{
	char name[20];    //函数名
	int address;
	bool haveReturnValue;   //是否有返回值
	bool havePara;
	struct funItem* next;
}funItem;


typedef struct Code    //中间代码
{
	char opt[10];
	int operand;
}Code;
