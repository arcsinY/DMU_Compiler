#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "var.h"
#define maxsymbolIndex 23

Code codes[200];
varItem* varSymbol[maxsymbolIndex];
funItem* funSymbol[maxsymbolIndex];
int codesIndex = 0;
int globeVar = 0;    //全局变量个数
int varSymbolIndex = 0;
int funSymbolIndex = 0;
int offset = 0;
int es = 0;
int switch_end;
int loop, loop_end,loop_next;   //loop标志是否是循环体，1为是 0为否,loop_end标志循环体结束位置 ,loop_next标志continue后下次到哪里
int returnCx[10] = { 0 };      //return指令在中间代码的位置
int returnCxIndex = 0;
int returnValue = 0;  //要返回的值
int globeVarInitCX = 0;     //全局变量的初始化指令从第几条中间代码开使
int len_globeVarInitCX = 0;     //全局变量的初始化指令一共有几条
bool startGlobeCode = true;    //是否为第一条全局变量初始化中间代码
int cxEnter = 0;      //Enter指令在中间代码的位置
bool haveRet = false;
char nowFun[20];      //目前正在处理的函数
char token[20], token1[40];    //单词类别值、单词自身值
char tokenfile[30];   //单词流文件的名字
char Codeout[30];     //中间代码文件名
FILE *fpTokenin;      //单词流文件指针
FILE *fpCodeBinary;   //中间代码二进制文件指针
FILE *fpCodeText;     //中间代码文本文件指针
extern char scanOut[300];


int TESTparse();
int program();
int fun_declaration();
int main_declaration();
int function_body();
int compound_stat();
int statement();
int expression_stat();
int expression();
int bool_expr();
int additive_expr();
int term();
int factor();
int if_stat();
int dowhile_stat();
int while_stat();
int break_stat();
int continue_stat();
int for_stat();
int call_stat();   //call ID();  时使用
int call_expr();   //ID = call ID()时使用
int switch_stat();
int case_stat(char* temp);
int default_stat(char* temp);
int return_stat();
int write_stat();
int read_stat();
int declaration_stat();
int declaration_list();
int globeDeclaration_list();
int globeDeclaration_stat();
int statement_list();
int compound_stat();
int strToASCII(char * s);
void initSymbol();
int insert_varSymbol(char *funName, char *name, varKind kind, int size);
int insert_funSymbol(char *name, bool haveRet);
int lookup_varSymbol(char *name, int &address, char* funName, varKind kind, int bias);
int lookup_funSymbol(char* name, int &address, bool &haveRet, bool &havePara);
void traverseVarLinkList(varItem* H);
void traverseFunLinkList(funItem* H);
void outputVarSymbol();
void outputFunSymbol();
void writeAddressForFun(char* name, int address);
int parameter_list();     //function参数接收
int parameter_stat();
void setHavePara(char* funName, bool havePara);    //为名为funName的函数设置是否有参数

int strToASCII(char * s)    //字符串转化为一个数值
{
	int ascii = 0;
	for (int i = 0; i < strlen(s); ++i)
		ascii += s[i];
	return ascii;
}

void initSymbol()    //初始化符号表
{
	for (int i = 0; i < maxsymbolIndex; ++i)
	{
		varSymbol[i] = (varItem*)malloc(sizeof(varItem));
		varSymbol[i]->next = NULL;
		funSymbol[i] = (funItem*)malloc(sizeof(funItem));
		funSymbol[i]->next = NULL;
	}
}


int insert_varSymbol(char *funName, char *name, varKind kind, int size)   //变量插入符号表
//参数：所在函数名，变量名，类型（普通变量/数组），大小（普通为1，数组为数组大小）
{
	varItem* thisVar = (varItem*)malloc(sizeof(varItem));
	varSymbolIndex = strToASCII(name) % maxsymbolIndex;    //计算应该放在哪个单元
	/********设置变量属性********/
	if (kind != globe)   //不是全局变量，存储所在函数名
		strcpy(thisVar->inFunc, funName);
	strcpy(thisVar->name, name);    //存储变量名
	thisVar->kind = kind;
	if (kind == globe)     //全局变量的地址不是offset
		thisVar->address = globeVar;
	else
		thisVar->address = offset;
	thisVar->next = NULL;
	/************************/
	if(kind != globe)
		offset += size;
	varItem* p = varSymbol[varSymbolIndex]->next;
	varItem* q = varSymbol[varSymbolIndex];
	while (p)
	{
		if ((strcmp(thisVar->name, p->name) == 0 && strcmp(thisVar->inFunc, p->inFunc) == 0)
			|| (strcmp(thisVar->name, p->name) == 0 && thisVar->kind == globe) 
			|| (strcmp(thisVar->name, p->name) == 0 && p->kind == globe))
		{     //变量重复声明:变量名相同且所属函数相同。变量名相同且有一个是全局变量
			es = 22;
			return es;
		}
		p = p->next;
		q = q->next;
	}
	q->next = thisVar;
	strcpy(token1, name);
	return 0;
}


int insert_funSymbol(char *name, bool haveRet)    //函数插入符号表
{
	funItem* thisFun = (funItem*)malloc(sizeof(funItem));
	funSymbolIndex = strToASCII(name) % maxsymbolIndex;
	strcpy(thisFun->name, name);
	thisFun->haveReturnValue = haveRet;
	thisFun->next = NULL;
	funItem* p = funSymbol[funSymbolIndex];
	while (p->next)
	{
		if (strcmp(p->name, thisFun->name) == 0)
		{
			es = 32;    //函数重复定义
			return es;
		}
		p = p->next;
	}
	p->next = thisFun;
	return 0;
}


int lookup_varSymbol(char *name, int &address, char* funName, varKind kind, int bias)    //查变量符号表
{   //参数：变量名，存地址的变量，变量所在函数名，变量类型（普通/数组/全局），偏移量（数组要指定，普通变量随意）
	int i = strToASCII(name) % maxsymbolIndex;
	varItem* p = varSymbol[i]->next;
	if (kind == globe)
	{
		while (p)
		{
			if (strcmp(p->name, name) == 0)      //只考虑变量名
			{
				address = p->address;
				return 0;
			}
			p = p->next;
		}
	}
	else
	{
		while (p)
		{
			if (strcmp(p->name, name) == 0 && strcmp(p->inFunc, funName) == 0)   //考虑变量名和所在函数名
			{
				if (kind == array)
					address = p->address + bias;
				else
					address = p->address;
				return 0;
			}
			p = p->next;
		}
	}
	es = 23;
	return es;
}


int lookup_funSymbol(char* name, int &address, bool &haveRet, bool &havePara)
{
	int i = strToASCII(name) % maxsymbolIndex;
	funItem* p = funSymbol[i]->next;
	while (p)
	{
		if (strcmp(p->name, name) == 0)
		{
			address = p->address;
			haveRet = p->haveReturnValue;
			havePara = p->havePara;
			return 0;
		}
		p = p->next;
	}
	es = 33;
	return es;
}


void writeAddressForFun(char* name, int address)
{
	int i = strToASCII(name) % maxsymbolIndex;
	funItem* p = funSymbol[i]->next;
	while (p)
	{
		if (strcmp(p->name, name) == 0)
			p->address = address;
		p = p->next;
	}
}

void setHavePara(char* funName, bool havePara)
{
	int i = strToASCII(funName) % maxsymbolIndex;
	funItem* p = funSymbol[i]->next;
	while (p)
	{
		if (strcmp(p->name, funName) == 0)
		{
			p->havePara = havePara;
		}
		p = p->next;
	}
}

void traverseVarLinkList(varItem* H)    //遍历变量符号表的链表
{
	varItem* p = H->next;
	while (p)
	{
		if(p->kind != globe)
			printf("%-10s%-10s", p->name, p->inFunc);
		else
			printf("%-10s%-10s", p->name, "null");
		if (p->kind == variable)
			printf("variable\n");
		else if(p->kind == array)
			printf("array\n");
		else
			printf("global\n");
		p = p->next;
	}
}


void traverseFunLinkList(funItem* H)         //遍历函数符号表的链表
{
	funItem* p = H->next;
	while (p)
	{
		printf("%-10s%-3d\n", p->name, p->address);
		p = p->next;
	}
}


void outputVarSymbol()       //输出所有定义的变量，包括变量名，所在函数，类型
{
	for (int i = 0; i < maxsymbolIndex; ++i)
		traverseVarLinkList(varSymbol[i]);
}


void outputFunSymbol()       //输出定义的所有函数，包括函数名和入口地址
{
	for (int i = 0; i < maxsymbolIndex; ++i)
		traverseFunLinkList(funSymbol[i]);
}


int TESTparse()
{
	initSymbol();
	codesIndex = 0;
	int i;
	int es = 0;
	if ((fpTokenin = fopen(scanOut, "r")) == NULL)
	{
		printf("\n打开单词流文件错误!\n");
		es = 10;
		return es;
	}
	es = program();
	fclose(fpTokenin);
	printf("==语法、语义分析及代码生成程序结果==\n");
	switch (es)
	{
		case 0:
			printf("语法、语义分析成功并抽象机汇编生成代码!\n");
			break;
		case 10:
			printf("打开文件 %s失败!\n", tokenfile);
			break;
		case 1:
			printf("缺少{!\n");
			break;
		case 2:
			printf("缺少}!\n");
			break;
		case 3:
			printf("缺少标识符!\n");
			break;
		case 4:
			printf("少分号!\n");
			break;
		case 5:
			printf("缺少(!\n");
			break;
		case 6:
			printf("缺少)!\n");
			break;
		case 7: 
			printf("缺少操作数!\n");
			break;
        case 8: 
			printf("缺少逗号\n");
            break;
		case 9:
			printf("数组下标必须为常量\n");
			break;
		case 11:
			printf("函数开头缺少{!\n");
			break;
		case 12:
			printf("函数结束缺少}!\n");
			break;
		case 13:
			printf("最后一个函数的名字应该是main}!\n");
			break;
		case 14:
			printf("缺少]\n");
			break;
		case 15:
			printf("变量声明语句不合法\n");
			break;
		case 16:
			printf("函数未声明返回值类型\n");
			break;
		case 17:
			printf("函数没有返回值\n");
			break;
		case 21:
			printf("符号表溢出!\n");
			break;
		case 22:
			printf("变量%s重复定义!\n", token1);
			break;
		case 23:
			printf("变量未声明!\n");
			break;
		case 24:
			printf("程序中main函数结束后，还有其它多余字符\n");
			break;
		case 32:
			printf("函数重复定义!\n");
			break;
		case 33:
			printf("函数未定义\n");
			break;
		case 34:
			printf("call语句后面的标识符%s不是变量名!\n", token1);
			break;
		case 35:
			printf("read语句后面的标识符不是变量名!\n");
			break;
		case 36:
			printf("赋值语句的左值%s不是变量名!\n", token1);
			break;
		case 37:
			printf("因子对应的标识符不是变量名!\n");
			break;
		case 40:
			printf("do-while语句不完整\n");
			break;
		case 51:
			printf("switch后的表达式不是变量\n");
			break;
		case 53:
			printf("case后必须为常数\n");
			break;
		case 54:
			printf("缺少冒号\n");
			break;
		case 55:
			printf("缺少break\n");
			break;
		default:
			;
	}
	//输出符号表的内容
	printf("变量符号表\n");
	printf("名字\t所属函数   变量类型\n");
	outputVarSymbol();
	printf("函数名\t入口地址\n");
	outputFunSymbol();
	printf("请输入要生成的文本形式的中间代码文件的名字（包括路径）：");
	scanf("%s", Codeout);
	if ((fpCodeText = fopen(Codeout, "w")) == NULL)
	{
		printf("\n创建%s错误!\n", Codeout);
		es = 10;
		return es;
	}
	for (i = 0; i<codesIndex; i++)
	{
		if (strcmp(codes[i].opt, "LOAD") == 0 || strcmp(codes[i].opt, "LOADI") == 0 || strcmp(codes[i].opt, "STO") == 0 ||
			strcmp(codes[i].opt, "BR") == 0 || strcmp(codes[i].opt, "BRF") == 0 || strcmp(codes[i].opt, "CAL") == 0 || strcmp(codes[i].opt, "ENTER") == 0)
			fprintf(fpCodeText, "%3d %-5s %d\n", i, codes[i].opt, codes[i].operand);
		else
			fprintf(fpCodeText, "%3d %-5s\n", i, codes[i].opt);
	}
	fclose(fpCodeText);
	printf("请输入要生成的二进制形式的中间代码文件的名字（结构体存储）:");
	scanf("%s", Codeout);
	if ((fpCodeBinary = fopen(Codeout, "wb")) == NULL)
	{
		printf("\n创建%s错误!\n", Codeout);
		es = 10;
		return es;
	}
	fwrite(codes, sizeof(Code), codesIndex, fpCodeBinary);
	fclose(fpCodeBinary);
	return es;
}

//<program> →{ fun_declaration }<main_declaration>
//<fun_declaration> → function ID’(‘ ‘ )’< function_body>
//<main_declaration>→  main’(‘ ‘ )’ < function_body>
int program()
{
	int es = 0, i;

	strcpy(codes[codesIndex++].opt, "BR");     //codes数组的第一条指令是无条件转移到main函数的入口，入口地址需要返填
	
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = globeDeclaration_list();
	if (es > 0)
		return es;
	while (strcmp(token, "function") == 0)  //判断是否是函数定义
	{
		fscanf(fpTokenin, "%s %s\n", token, token1);
		es = fun_declaration();
		if (es != 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", token, token1);
	}
	if (strcmp(token, "ID"))  //整个程序的最后是main函数的定义，其类别值为“ID”
	{
		es = 1;
		return es;
	}
	if (strcmp(token1, "main"))
	{
		es = 13;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = main_declaration();
	if (es > 0)
		return es;
	return es;
}

//<fun_declaration> → function void|int ID’(‘ ‘ )’< function_body>
int fun_declaration()
{
	int es = 0;
	int cx1, i, j;

	if (strcmp(token, "void") && strcmp(token, "int"))    //不是void也不是int
	{
		es = 16;
		return es;
	}
	if (strcmp(token, "void") == 0)   
		haveRet = false;
	else
		haveRet = true;
	fscanf(fpTokenin, "%s %s\n", token, token1);
	if (strcmp(token, "ID"))
	{
		es = 3;
		return es;
	}
	strcpy(nowFun, token1);
	insert_funSymbol(nowFun, haveRet);
	writeAddressForFun(nowFun, codesIndex);
	offset = 2;//进入一个新的函数，变量的相对地址从2开始
	strcpy(nowFun, token1);
	strcpy(codes[codesIndex].opt, "ENTER");//函数体的开始
	cx1 = codesIndex++;


	fscanf(fpTokenin, "%s %s\n", token, token1);
	if (strcmp(token, "("))
	{
		es = 5;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);   //读取参数
	es = parameter_list();

	j = codesIndex - 1;
	for (i = 2; i<offset; i++)
	{
		codes[j].operand = i;
		j = j - 2;
	}

	if(es>0) 
		return es;
	if (strcmp(token, ")"))
	{
		es = 6;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = function_body();
	codes[cx1].operand = offset;
	return es;
}

//<main_declaration>→ main’(‘ ‘ )’ < function_body>
int main_declaration()
{
	int es = 0;
	int cx1;
	
	insert_funSymbol("main", false);
	strcpy(nowFun, "main");
	writeAddressForFun(nowFun, codesIndex);//把函数体的入口地址填入函数名在符号表中的地址字段
	offset = 2;//进入一个新的函数，变量的相对地址从2开始
	strcpy(codes[codesIndex].opt, "ENTER");//函数体的开始
	codes[0].operand = codesIndex;
	cx1 = codesIndex++;

	if (strcmp(token, "("))
	{
		es = 5;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);
	if (strcmp(token, ")"))
	{
		es = 6;
		return es;
	}
	
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = function_body(); 
	codes[cx1].operand = offset;
	return es;
}

//<function_body>→'{'<declaration_list><statement_list>'}'
int function_body()
{
	int es = 0;

	if (strcmp(token, "{")) //判断是否'{'
	{
		es = 11;
		return es;
	}
	
	//offset = 2;    //进入一个新的函数，变量的相对地址从2开始
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = declaration_list();
	if (es>0)
		return es;
	
	es = statement_list();
	if (es>0)
		return es;
	if (strcmp(token, "}")) //判断是否'}'
	{
		es = 12;
		return es;
	}
	strcpy(codes[codesIndex].opt, "RETURN");//函数体的结束
	for (int i = 0; i < returnCxIndex; ++i)   //反填函数中return语句的跳转位置
	{
		codes[returnCx[i]].operand = codesIndex;
	}
	returnCxIndex = 0;
	++codesIndex;
	return es;
}

//<declaration_list>→{<declaration_stat>}
int declaration_list()
{
	int es = 0;
	while (strcmp(token, "int") == 0) 
	{
		es = declaration_stat();
		//fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (es > 0) 
			return es;
	}
	return es;
}


int declaration_stat()    //可以声明的同时赋值，可连续声明，可声明数组
{
	int es = 0;
	char name[30];   //存放变量名
	fscanf(fpTokenin, "%s %s\n", &token, &token1);   
	if (strcmp(token, "ID"))   //读的第一个单词必须是标识符，因为进入这个函数之前一个单词是，或int
	{
		es = 3;
		return es;
	}
	strcpy(name, token1);    //保存当前标识符名字
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ",") == 0)   //标识符之后是逗号,说明这个标识符是普通变量，插入符号表。之后继续声明变量
	{
		es = insert_varSymbol(nowFun, name, variable, 1);
		if (es > 0)
			return es;
		es = declaration_stat();
		return es;
	}
	else if (strcmp(token, ";") == 0)   //之后是分号，语句结束
	{
		es = insert_varSymbol(nowFun, name, variable, 1);
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "=") == 0)  //之后是等号，要对普通变量赋值
	{
		es = insert_varSymbol(nowFun, name, variable, 1); 
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);   //再往后读一个
		if (strcmp(token, "NUM") == 0)   //常量给变量赋值
		{
			strcpy(codes[codesIndex].opt, "LOADI");
			codes[codesIndex++].operand = atoi(token1);
			strcpy(codes[codesIndex].opt, "STO");
			int address;
			es = lookup_varSymbol(name, address, nowFun, variable, 0);
			codes[codesIndex++].operand = address;
		}
		else if(strcmp(token, "ID") == 0)      //变量给变量赋值
		{
			int address;
			es = lookup_varSymbol(token1, address, nowFun, variable, 0);
			if (es > 0)   //变量找不到，还可能是全局变量
			{
				es = lookup_varSymbol(token1, address, NULL, globe, 0);
				if (es > 0)
					return es;
				strcpy(codes[codesIndex].opt, "LOADA");   //全局变量对变量赋值
				codes[codesIndex++].operand = address;
				strcpy(codes[codesIndex].opt, "STO");
				es = lookup_varSymbol(name, address, nowFun, variable, 0);
				codes[codesIndex++].operand = address;
			}
			else    //普通变量对变量赋值
			{
				strcpy(codes[codesIndex].opt, "LOAD");
				codes[codesIndex++].operand = address;
				strcpy(codes[codesIndex].opt, "STO");
				es = lookup_varSymbol(name, address, nowFun, variable, 0);
				codes[codesIndex++].operand = address;
			}
		}
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if(strcmp(token, ",") == 0)      //之后是逗号，继续声明
			es = declaration_stat();
		if (strcmp(token, ";") == 0)
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "[") == 0)   //这是个数组
	{
		int size = 0;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "NUM"))    //刚读入的是数组大小，必须为常量
		{
			es = 9;
			return es;
		}
		size = atoi(token1);
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "]"))
		{
			es = 14;
			return es;
		}
		es = insert_varSymbol(nowFun, name, array, size);
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, ",") == 0)      //之后是逗号，继续声明
			es = declaration_stat();
		if (strcmp(token, ";") == 0)
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else
	{
		es = 15;
		return es;
	}
	return es;
}

//<statement_list>→{<statement>}
int statement_list()
{
	int es = 0;
	while (strcmp(token, "}" ))
	{
		es = statement();
		if (es > 0) 
			return es;
	}
	return es;
}

//<statement>→ <if_stat>|<while_stat>|<for_stat>
//               |<compound_stat> |<expression_stat>| <call _stat>
//int statement()
//{
//	int es = 0;
//	if (es == 0 && strcmp(token, "if") == 0) es = if_stat(); //<if语句>
//	if (es == 0 && strcmp(token, "while") == 0) es = while_stat(); //<while语句>
//	if (es == 0 && strcmp(token, "for") == 0) es = for_stat(); //<for语句>
//	if (es == 0 && strcmp(token, "do") == 0) es = dowhile_stat(); 												   //可在此处添加do语句调用
//	if (es == 0 && strcmp(token, "read") == 0) es = read_stat(); //<read语句>
//	if (es == 0 && strcmp(token, "write") == 0) es = write_stat(); //<write语句>
//	if (es == 0 && strcmp(token, "{") == 0) es = compound_stat(); //<复合语句>
//	if (es == 0 && strcmp(token, "call") == 0) es = call_stat();//<函数调用语句>
//	if (es == 0 && (strcmp(token, "ID") == 0 || strcmp(token, "NUM") == 0 || strcmp(token, "(") == 0)) es = expression_stat(); //<表达式语句>
//	if (es == 0 && strcmp(token, "return") == 0) es = return_stat();  //<函数返回语句>
//	if (es == 0 && strcmp(token, "switch") == 0) es = switch_stat();
//	if (es == 0 && strcmp(token, "continue") == 0) es = continue_stat();
//	if (es == 0 && strcmp(token, "break") == 0) es = break_stat();
//	return es;
//}

int statement()
{
	int es = 0;
	if (es == 0 && strcmp(token, "if") == 0) es = if_stat(); //<if语句>
	else if (es == 0 && strcmp(token, "while") == 0) es = while_stat(); //<while语句>
	else if (es == 0 && strcmp(token, "for") == 0) es = for_stat(); //<for语句>
	else if (es == 0 && strcmp(token, "do") == 0) es = dowhile_stat(); 												   //可在此处添加do语句调用
	else if (es == 0 && strcmp(token, "read") == 0) es = read_stat(); //<read语句>
	else if (es == 0 && strcmp(token, "write") == 0) es = write_stat(); //<write语句>
	else if (es == 0 && strcmp(token, "{") == 0) es = compound_stat(); //<复合语句>
	else if (es == 0 && strcmp(token, "call") == 0) es = call_stat();//<函数调用语句>
	else if (es == 0 && (strcmp(token, "ID") == 0 || strcmp(token, "NUM") == 0 || strcmp(token, "(") == 0)) es = expression_stat(); //<表达式语句>
	else if (es == 0 && strcmp(token, "return") == 0) es = return_stat();  //<函数返回语句>
	else if (es == 0 && strcmp(token, "switch") == 0) es = switch_stat();
	else if (es == 0 && strcmp(token, "continue") == 0) es = continue_stat();
	else if (es == 0 && strcmp(token, "break") == 0) es = break_stat();
	return es;
}

//<if_stat>→ if '('<expr>')' <statement > [else < statement >]
int if_stat()
{
	int es = 0, cx1, cx2; //if
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))
		return es = 5; //少左括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ")"))
		return es = 6; //少右括号
	strcpy(codes[codesIndex].opt, "BRF");
	cx1 = codesIndex++;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es > 0)
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	cx2 = codesIndex++;
	codes[cx1].operand = codesIndex;
	if (strcmp(token, "else") == 0) //else部分处理
	{
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = statement();
		if (es > 0)
			return es;
	}
	codes[cx2].operand = codesIndex;
	return es;
}

//<while_stat>→ while '('<expr >')' < statement >
int while_stat()
{
	loop = 1;
	int es = 0;
	int cx1, cxEntrance;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  return(es = 5); //少左括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	cxEntrance = codesIndex;
	loop_next = cxEntrance;//为了continue回到下一次循环用
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ")"))
		return es = 6; //少右括号
	strcpy(codes[codesIndex].opt, "BRF");
	cx1 = codesIndex++;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es > 0)
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex].operand = cxEntrance;
	codesIndex++;
	codes[cx1].operand = codesIndex;
	if (loop_end != 0) 
	{
		//	反填
		codes[loop_end].operand = codesIndex;
		loop_end = 0;

	}
	loop = 0;//标志退出循环体	
	loop_next = 0;//标志退出循环
	return es;
}

//<for_stat>→ for'('<expr>,<expr>,<expr>')'<statement>
int for_stat()
{
	loop = 1;
	int es = 0;
	int cx1, cx2, cxExp2, cxExp3;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  return(es = 5); //少左括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	es = expression();

	if (es > 0) 
		return es;
	if (strcmp(token, ";")) 	
		return(es = 4); //少分号
	cxExp2 = codesIndex;
	loop_next = cxExp2;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0) 
		return es;
	strcpy(codes[codesIndex].opt, "BRF");
	cx1 = codesIndex++;
	strcpy(codes[codesIndex].opt, "BR");
	cx2 = codesIndex++;
	if (strcmp(token, ";"))  
		return(es = 4); //少分号
	cxExp3 = codesIndex;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0) 
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex].operand = cxExp2;
	codesIndex++;
	codes[cx2].operand = codesIndex;
	if (strcmp(token, ")"))  return(es = 6); //少右括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es > 0) 
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex].operand = cxExp3;
	codesIndex++;
	codes[cx1].operand = codesIndex;
	if (loop_end != 0) 
	{
		//	反填
		codes[loop_end].operand = codesIndex;
		loop_end = 0;

	}
	loop = 0;//标志退出循环体
	loop_next = 0;
	return es;
}

//<write_stat>→write <expression>;
int write_stat()
{
	int es = 0;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ";"))  
		return (es = 4); //少分号
	strcpy(codes[codesIndex++].opt, "OUT");
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//<read_stat>→read ID;
int read_stat()
{
	int es = 0, address;
	char name[30];
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	strcpy(name, token1);   //存变量名
	if (strcmp(token, "ID"))  
		return(es = 3); //少标识符
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "[") == 0)   //数组
	{
		int index = 0;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		index = atoi(token1);
		if (strcmp(token, "NUM"))  //下标
		{
			es = 9;
			return es;
		}
		es = lookup_varSymbol(name, address, nowFun, array, index);
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "["))
		{
			es = 14;
			return es;
		}
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
	}
	else
		es = lookup_varSymbol(name, address, nowFun, variable, 0);
	if (es > 0)
	{
		es = lookup_varSymbol(name, address, NULL, globe, 0);  //read 全局变量
		if (es > 0)
			return es;
		strcpy(codes[codesIndex++].opt, "IN"); 
		strcpy(codes[codesIndex].opt, "STOA");
		codes[codesIndex++].operand = address;
	}
	else
	{
		strcpy(codes[codesIndex++].opt, "IN");
		strcpy(codes[codesIndex].opt, "STO");
		codes[codesIndex++].operand = address;
	}
	if (strcmp(token, ";"))  
		return(es = 4); //少分号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//<compound_stat>→'{'<statement_list>'}'
int compound_stat()    //复合语句函数
{
	int es = 0;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement_list();
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//< call _stat>→call ID'(' callparameter_list ')'
int call_stat()
{
	int es = 0, address;
	bool havePara;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	if (strcmp(token, "ID")) 
		return (es = 3); //少标识符

	es = lookup_funSymbol(token1, address, haveRet, havePara);
	if (es > 0) 
		return es;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  
		return (es = 5); //少(
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = parameter_list();
	if (es > 0) 
		return es;
	if (strcmp(token, ")"))  
		return(es = 6); //少)
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";")) 
		return(es = 4); //少分号

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	strcpy(codes[codesIndex].opt, "CAL");
	codes[codesIndex++].operand = address;
	return es;
}

//<expression_stat>→<expression>;|;
int expression_stat()
{
	int es = 0;
	if (strcmp(token, ";") == 0)
	{
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	es = expression();
	if (es > 0) return es;
	if (strcmp(token, ";") == 0) 
	{
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else 
	{
		es = 4;
		return es;//少分号
	}
}

//<expression>→ ID=<bool_expr>|<bool_expr>|ID=call_stat()
int expression()       
{
	int es = 0, fileadd;
	bool isGlobe = false;   //被赋值的变量是否为全局变量
	bool haveAddress = false;
	char token2[20], token3[40];
	int address;
	if (strcmp(token, "ID") == 0) 
	{
		fileadd = ftell(fpTokenin); //记住当前文件位置
		fscanf(fpTokenin, "%s %s\n", &token2, &token3);
		if (strcmp(token2, "[") == 0)   //这是个数组
		{
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);  
			if (strcmp(token2, "NUM"))    //读数组下标
			{
				es = 9;
				return es;
			}
			es = lookup_varSymbol(token1, address, nowFun, array, atoi(token3));
			haveAddress = true;    //取得数组元素的地址
			if (es > 0)
				return es;
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);
			if (strcmp(token2, "]"))
			{
				es = 14;
				return es;
			}
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);
		}
		if (strcmp(token2, "=") == 0) //要对变量赋值
		{
			if(!haveAddress)   //如果变量不是数组，则要取地址
				es = lookup_varSymbol(token1, address, nowFun, variable, 0);
			if (es > 0)   //对全局变量赋值
			{
				es = lookup_varSymbol(token1, address, NULL, globe, 0);
				isGlobe = true;
				if (es > 0)
					return es;
			}
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
			if (strcmp(token, "call") == 0)    //用函数的返回值赋值
			{
				es = call_expr();
			}
			else
				es = bool_expr();  //计算要赋的值
			if (es > 0)
				return es;
			if(isGlobe == false)
				strcpy(codes[codesIndex].opt, "STO");
			else
				strcpy(codes[codesIndex].opt, "STOA");
			codes[codesIndex++].operand = address;
		}
		else 
		{
			fseek(fpTokenin, fileadd, 0); //若非'='则文件指针回到'='前的标识符
			es = bool_expr();
			if (es > 0) 
				return es;
		}
	}
	else 
		es = bool_expr();
	return es;
}



//<bool_expr>-><additive_expr> |< additive_expr >(>|<|>=|<=|==|!=)< additive_expr >
int bool_expr()
{
	int es = 0;
	es = additive_expr();
	if (es > 0) 
		return es;
	if (strcmp(token, ">") == 0 || strcmp(token, ">=") == 0
		|| strcmp(token, "<") == 0 || strcmp(token, "<=") == 0
		|| strcmp(token, "==") == 0 || strcmp(token, "!=") == 0
		||strcmp(token, "&&") == 0 || strcmp(token, "||") == 0)
	{
		char token2[20];
		strcpy(token2, token);      //保存运算符
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = additive_expr();
		if (es > 0) return es;
		if (strcmp(token2, ">") == 0)
			strcpy(codes[codesIndex++].opt, "GT");
		if (strcmp(token2, ">=") == 0)
			strcpy(codes[codesIndex++].opt, "GE");
		if (strcmp(token2, "<") == 0)
			strcpy(codes[codesIndex++].opt, "LES");
		if (strcmp(token2, "<=") == 0)
			strcpy(codes[codesIndex++].opt, "LE");
		if (strcmp(token2, "==") == 0)
			strcpy(codes[codesIndex++].opt, "EQ");
		if (strcmp(token2, "!=") == 0)
			strcpy(codes[codesIndex++].opt, "NOTEQ");
		if (strcmp(token2, "&&") == 0)   //与、或、非运算
			strcpy(codes[codesIndex++].opt, "AND");
		if (strcmp(token2, "||") == 0)
			strcpy(codes[codesIndex++].opt, "OR");
		if(strcmp(token2, "!") == 0)
			strcpy(codes[codesIndex++].opt, "NOT");
	}
	return es;
}

//<additive_expr>→<term>{(+|-)< term >}
int additive_expr()
{
	int es = 0;
	es = term();
	if (es > 0)
		return es;
	while (strcmp(token, "+") == 0 || strcmp(token, "-") == 0)
	{
		char token2[20];
		strcpy(token2, token);
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = term();
		if (es > 0) return es;
		if (strcmp(token2, "+") == 0)
			strcpy(codes[codesIndex++].opt, "ADD");
		if (strcmp(token2, "-") == 0)
			strcpy(codes[codesIndex++].opt, "SUB");
	}
	return es;
}

//< term >→<factor>{(*| /)< factor >}
int term()
{
	int es = 0;
	es = factor();
	if (es > 0)
		return es;
	while (strcmp(token, "*") == 0 || strcmp(token, "/") == 0) 
	{
		char token2[20];
		strcpy(token2, token);
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = factor();
		if (es > 0)
			return es;
		if (strcmp(token2, "*") == 0)
			strcpy(codes[codesIndex++].opt, "MULT");
		if (strcmp(token2, "/") == 0)
			strcpy(codes[codesIndex++].opt, "DIV");
	}
	return es;
}

//< factor >→'('<additive_expr>')'| ID|NUM
int factor()
{
	int es = 0;
	if (strcmp(token, "(") == 0)
	{
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = expression();
		if (es > 0)
			return es;
		if (strcmp(token, ")"))
			return es = 6;    //少右括号
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
	}
	else    
	{
		char token2[20], token3[30];
		int address;
		if (strcmp(token, "ID") == 0)
		{
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);
			if (strcmp(token2, "[") == 0)   //这是个数组
			{
				fscanf(fpTokenin, "%s %s\n", &token2, &token3);
				if (strcmp(token2, "NUM"))
				{
					es = 9;
					return es;
				}
				int index = atoi(token3);
				es = lookup_varSymbol(token1, address, nowFun, array, index);  //得到地址
				if (es > 0)
					return es;
				fscanf(fpTokenin, "%s %s\n", &token2, &token3);
				if (strcmp(token2, "]"))
				{
					es = 14;
					return es;
				}
				fscanf(fpTokenin, "%s %s\n", &token, &token1);
			}
			else
			{
				es = lookup_varSymbol(token1, address, nowFun, variable, 0); //查符号表，获取变量地址
				if (es == 0)
				{
					strcpy(token, token2);
					strcpy(token1, token3);
				}
			}
			if (es > 0)
			{
				es = lookup_varSymbol(token1, address, NULL, globe, 0);
				if (es > 0)
					return es;
				strcpy(codes[codesIndex].opt, "LOADA");
				codes[codesIndex++].operand = address;
				strcpy(token, token2);
				strcpy(token1, token3);
			}
			else
			{
				strcpy(codes[codesIndex].opt, "LOAD");
				codes[codesIndex++].operand = address;
			}
			return es;
		}
		if (strcmp(token, "NUM") == 0)
		{
			strcpy(codes[codesIndex].opt, "LOADI");
			codes[codesIndex++].operand = atoi(token1);
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
			return es;
		}
		else 
		{
			es = 7; //缺少操作数
			return es;
		}
	}
	return es;
}


//<parameter_list>->{<parameter_stat>}
int parameter_list()
{
	int es = 0;
	if (strcmp(token, ")") == 0)    //没有参数
	{
		setHavePara(nowFun, false);
		return es;
	}
	//fscanf(fpTokenin, "%s %s\n", token, token1);
	if (strcmp(token, "int") == 0) 
	{
		setHavePara(nowFun, true);
		es = parameter_stat();
		if (es>0) 
			return es;
		while (strcmp(token, ",") == 0)
		{
			fscanf(fpTokenin, "%s %s\n", token, token1);
			if (strcmp(token, "int") == 0)
			{
				es = parameter_stat();
				if (es>0) 
					return es;
			}
			else 
				return es = 0;
		}
	}
	return es;
}



int parameter_stat()
{
	int es = 0;
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = insert_varSymbol(nowFun, token1, variable, 1);
	if (es>0) 
		return (es);
	strcpy(codes[codesIndex++].opt, "POP");
	strcpy(codes[codesIndex++].opt, "STO");
	fscanf(fpTokenin, "%s %s\n", token, token1);
	return es;
}



//<return_stat>->return bool_expr()
int return_stat() 
{
	int es = 0;
	if (haveRet == false)
	{
		es = 17;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = additive_expr();   //调用算术表达式语句
	if (es>0) 
		return es;
	if (strcmp(token, ";")) 
		return (es = 4);
	strcpy(codes[codesIndex++].opt, "PUSH");
	strcpy(codes[codesIndex].opt, "BR");    //遇到return语句后跳转到改函数结束的位置，操作数反填
	returnCx[returnCxIndex++] = codesIndex++;  //保存这条BR指令在中间代码的位置
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}


int call_expr()   //这样调用的函数必须有返回值
{
		int es = 0, address;
		bool havePara;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "ID"))
			return (es = 3); //少标识符
		es = lookup_funSymbol(token1, address, haveRet, havePara);

		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "("))
			return (es = 5); //少(
		if (havePara)   //有参数
		{
			do
			{
				fscanf(fpTokenin, "%s %s\n", &token, &token1);
				es = additive_expr();
				if (es > 0)
					return es;
				strcpy(codes[codesIndex++].opt, "PUSH");
			} while (strcmp(token, ",") == 0);
		}
		else
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, ")")) 
			return(es = 6);
		if (haveRet == false)
		{
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
			if (strcmp(token, ";")) 
				return(es = 4);
		}
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		strcpy(codes[codesIndex].opt, "CAL");
		codes[codesIndex++].operand = address;
		if (haveRet == true)
		{
			strcpy(codes[codesIndex++].opt, "POP");
		}
		return(es);
}

//<switch_case>-> '(' ID ')' <case_stat> <default_stat>
int switch_stat() 
{
	int es = 0; //
	char* temp = (char*)malloc(sizeof(char));
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))
		return es = 5; //少左括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "ID"))
		return es = 51; //switch暂定为只能变量
	strcpy(temp, token1);//记录变量值

	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	if (strcmp(token, ")"))
		return es = 6; //少右括号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);


	if (strcmp(token, "{"))
		return es = 1; //少左括号

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "case") == 0)
		es = case_stat(temp);
	if (es>0)
		return es;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "}"))
		return es = 2; //少右括号
}

//<case_stat>
int case_stat(char * temp)
{
	int es = 0, cx1, cx2, cx_break, address;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "NUM"))
		return es = 53;//case语句只能为常数

					   //判断输入变量和当前case值是否匹配
	strcpy(codes[codesIndex].opt, "LOADI");
	codes[codesIndex++].operand = atoi(token1);
	es = lookup_varSymbol(temp, address, nowFun, variable, 0);
	if (es > 0)
	{
		es = lookup_varSymbol(temp, address, NULL, globe, 0);
		strcpy(codes[codesIndex].opt, "LOADA");
		codes[codesIndex++].operand = address;
	}
	else
	{
		strcpy(codes[codesIndex].opt, "LOAD");
		codes[codesIndex++].operand = address;
	}
	strcpy(codes[codesIndex++].opt, "EQ");
	strcpy(codes[codesIndex].opt, "BRF");//如果不等于那么从这儿跳走
	cx2 = codesIndex++;//并记录当前BRF的位置
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ":"))
		return es = 54;//case语句后需要冒号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es>0)
		return es;

	if (strcmp(token, "break"))
		return es = 55; //少break
	strcpy(codes[codesIndex].opt, "BR");
	cx_break = codesIndex++;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //少分号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	codes[cx2].operand = codesIndex;
	if (strcmp(token, "case") == 0)
	{
		es = case_stat(temp);
		if (es>0)	
			return es;
		//本次case中没找到，到下一个case中找，上次匹配失败的BRF中的cx2将放到这儿的开头
	}
	else if (strcmp(token, "default") == 0) 
	{
		//所有的case语句都没有找到，那只能执行最后的default语句了
		es = default_stat(temp);
		if (es> 0)
			return es;
	}
	codes[cx_break].operand = switch_end;
	return es;

}

//<default_stat>
int default_stat(char * temp)
{
	int es = 0;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ":"))
		return es = 54;//default语句后需要冒号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es>0)
		return es;
	
	if (strcmp(token, "break"))
		return es = 55; //少break
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //少分号
	switch_end = codesIndex;
	return es;
}



int break_stat()
{
	int es = 0;
	if (loop == 0)
		return 0;
	strcpy(codes[codesIndex].opt, "BR");
	loop_end = codesIndex++;//我希望能直接跳到末尾，这里返回等待反填
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //少分号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

int continue_stat()
{
	if (loop_next == 0)
		return 0;

	int es = 0;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex++].operand = loop_next;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //少分号
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

int dowhile_stat() 
{
	/*在statement中加一条如果碰到do就到这儿的句子*/
	loop = 1;//标志进入循环体
	int es = 0;
	int cx, cx2;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	cx = codesIndex;
	es = statement();
	if (es>0)
		return es;
	if (strcmp(token, "while"))
		return (es = 40);//40代表do while语句中缺少while
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))
		return (es = 5);
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es>0)
		return es;
	if (strcmp(token, ")"))
		return (es = 6);
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return (es = 4);
	strcpy(codes[codesIndex].opt, "BRF");

	cx2 = codesIndex++;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex].operand = cx;
	loop_next = cx;
	codesIndex++;
	codes[cx2].operand = codesIndex;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (loop_end != 0)
	{      //	反填
		codes[loop_end].operand = codesIndex;
		loop_end = 0;
	}
	loop = 0;//标志退出循环体
	loop_next = 0;//标志退出循环
	return es;
}


int globeDeclaration_list()
{
	int es = 0;
	while (strcmp(token, "int") == 0)
	{
		es = globeDeclaration_stat();
		if (es > 0)
			return es;
	}
	return es;
}


int globeDeclaration_stat()
{
	int es = 0;
	char name[30];   //存放变量名
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "ID"))   //读的第一个单词必须是标识符，因为进入这个函数之前一个单词是，或int
	{
		es = 3;
		return es;
	}
	strcpy(name, token1);    //保存当前标识符名字
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ",") == 0)   //标识符之后是逗号,说明这个标识符是普通变量，插入符号表。之后继续声明变量
	{
		es = insert_varSymbol(NULL, name, globe, 1);    //全局变量第一个参数为NULL
		++globeVar;
		if (es > 0)
			return es;
		es = declaration_stat();
		return es;
	}
	else if (strcmp(token, ";") == 0)   //之后是分号，语句结束
	{
		es = insert_varSymbol(NULL, name, globe, 1);
		++globeVar;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "=") == 0)  //之后是等号，要对全局变量赋值
	{
		es = insert_varSymbol(NULL, name, globe, 1);
		++globeVar;
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);   //再往后读一个
		if (strcmp(token, "NUM") == 0)   //常量给变量赋值
		{
			strcpy(codes[codesIndex].opt, "LOADI");
			++len_globeVarInitCX;
			if (startGlobeCode == true)     //这是全局变量初始化代码的第一条，记录位置
			{
				startGlobeCode = false;
				globeVarInitCX = codesIndex; 
			}
			codes[codesIndex++].operand = atoi(token1);
			strcpy(codes[codesIndex].opt, "STOA");    //全局变量存数指令
			++len_globeVarInitCX;
			int address;
			es = lookup_varSymbol(name, address, NULL, globe, 0);
			codes[codesIndex++].operand = address;
		}
		else if (strcmp(token, "ID") == 0)      //全局变量给全局变量赋值
		{
			int address;
			es = lookup_varSymbol(token1, address, NULL, globe, 0);
			if (es > 0)
				return es;
			strcpy(codes[codesIndex].opt, "LOADA");   //全局变量取数指令
			++len_globeVarInitCX;
			if (startGlobeCode == true)     //这是全局变量初始化代码的第一条，记录位置
			{
				startGlobeCode = false;
				globeVarInitCX = codesIndex;
			}
			codes[codesIndex++].operand = address;
			
			strcpy(codes[codesIndex].opt, "STOA");
			++len_globeVarInitCX;
			es = lookup_varSymbol(name, address, NULL, globe, 0);
			codes[codesIndex++].operand = address;
		}
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, ",") == 0)      //之后是逗号，继续声明
			es = declaration_stat();
		if (strcmp(token, ";") == 0)
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	//全局变量不能为数组
	else
	{
		es = 15;
		return es;
	}
	return es;
}