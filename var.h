#pragma once
typedef struct err
{
	int lineNum;
	int es;
}err;


enum varKind { variable, array, globe };    //�������������ͣ���ͨ�����飬ȫ�ֱ�����


typedef struct varItem    //�������ű�
{
	char name[20];      //������
	char inFunc[20];     //���������ĺ�����
	enum varKind kind;
	int address;
	struct varItem* next;
}varItem;


typedef struct funItem     //�������ű�
{
	char name[20];    //������
	int address;
	bool haveReturnValue;   //�Ƿ��з���ֵ
	bool havePara;
	struct funItem* next;
}funItem;


typedef struct Code    //�м����
{
	char opt[10];
	int operand;
}Code;
