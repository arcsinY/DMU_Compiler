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
int globeVar = 0;    //ȫ�ֱ�������
int varSymbolIndex = 0;
int funSymbolIndex = 0;
int offset = 0;
int es = 0;
int switch_end;
int loop, loop_end,loop_next;   //loop��־�Ƿ���ѭ���壬1Ϊ�� 0Ϊ��,loop_end��־ѭ�������λ�� ,loop_next��־continue���´ε�����
int returnCx[10] = { 0 };      //returnָ�����м�����λ��
int returnCxIndex = 0;
int returnValue = 0;  //Ҫ���ص�ֵ
int globeVarInitCX = 0;     //ȫ�ֱ����ĳ�ʼ��ָ��ӵڼ����м���뿪ʹ
int len_globeVarInitCX = 0;     //ȫ�ֱ����ĳ�ʼ��ָ��һ���м���
bool startGlobeCode = true;    //�Ƿ�Ϊ��һ��ȫ�ֱ�����ʼ���м����
int cxEnter = 0;      //Enterָ�����м�����λ��
bool haveRet = false;
char nowFun[20];      //Ŀǰ���ڴ���ĺ���
char token[20], token1[40];    //�������ֵ����������ֵ
char tokenfile[30];   //�������ļ�������
char Codeout[30];     //�м�����ļ���
FILE *fpTokenin;      //�������ļ�ָ��
FILE *fpCodeBinary;   //�м����������ļ�ָ��
FILE *fpCodeText;     //�м�����ı��ļ�ָ��
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
int call_stat();   //call ID();  ʱʹ��
int call_expr();   //ID = call ID()ʱʹ��
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
int parameter_list();     //function��������
int parameter_stat();
void setHavePara(char* funName, bool havePara);    //Ϊ��ΪfunName�ĺ��������Ƿ��в���

int strToASCII(char * s)    //�ַ���ת��Ϊһ����ֵ
{
	int ascii = 0;
	for (int i = 0; i < strlen(s); ++i)
		ascii += s[i];
	return ascii;
}

void initSymbol()    //��ʼ�����ű�
{
	for (int i = 0; i < maxsymbolIndex; ++i)
	{
		varSymbol[i] = (varItem*)malloc(sizeof(varItem));
		varSymbol[i]->next = NULL;
		funSymbol[i] = (funItem*)malloc(sizeof(funItem));
		funSymbol[i]->next = NULL;
	}
}


int insert_varSymbol(char *funName, char *name, varKind kind, int size)   //����������ű�
//���������ں������������������ͣ���ͨ����/���飩����С����ͨΪ1������Ϊ�����С��
{
	varItem* thisVar = (varItem*)malloc(sizeof(varItem));
	varSymbolIndex = strToASCII(name) % maxsymbolIndex;    //����Ӧ�÷����ĸ���Ԫ
	/********���ñ�������********/
	if (kind != globe)   //����ȫ�ֱ������洢���ں�����
		strcpy(thisVar->inFunc, funName);
	strcpy(thisVar->name, name);    //�洢������
	thisVar->kind = kind;
	if (kind == globe)     //ȫ�ֱ����ĵ�ַ����offset
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
		{     //�����ظ�����:��������ͬ������������ͬ����������ͬ����һ����ȫ�ֱ���
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


int insert_funSymbol(char *name, bool haveRet)    //����������ű�
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
			es = 32;    //�����ظ�����
			return es;
		}
		p = p->next;
	}
	p->next = thisFun;
	return 0;
}


int lookup_varSymbol(char *name, int &address, char* funName, varKind kind, int bias)    //��������ű�
{   //�����������������ַ�ı������������ں��������������ͣ���ͨ/����/ȫ�֣���ƫ����������Ҫָ������ͨ�������⣩
	int i = strToASCII(name) % maxsymbolIndex;
	varItem* p = varSymbol[i]->next;
	if (kind == globe)
	{
		while (p)
		{
			if (strcmp(p->name, name) == 0)      //ֻ���Ǳ�����
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
			if (strcmp(p->name, name) == 0 && strcmp(p->inFunc, funName) == 0)   //���Ǳ����������ں�����
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

void traverseVarLinkList(varItem* H)    //�����������ű������
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


void traverseFunLinkList(funItem* H)         //�����������ű������
{
	funItem* p = H->next;
	while (p)
	{
		printf("%-10s%-3d\n", p->name, p->address);
		p = p->next;
	}
}


void outputVarSymbol()       //������ж���ı��������������������ں���������
{
	for (int i = 0; i < maxsymbolIndex; ++i)
		traverseVarLinkList(varSymbol[i]);
}


void outputFunSymbol()       //�����������к�������������������ڵ�ַ
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
		printf("\n�򿪵������ļ�����!\n");
		es = 10;
		return es;
	}
	es = program();
	fclose(fpTokenin);
	printf("==�﷨������������������ɳ�����==\n");
	switch (es)
	{
		case 0:
			printf("�﷨����������ɹ��������������ɴ���!\n");
			break;
		case 10:
			printf("���ļ� %sʧ��!\n", tokenfile);
			break;
		case 1:
			printf("ȱ��{!\n");
			break;
		case 2:
			printf("ȱ��}!\n");
			break;
		case 3:
			printf("ȱ�ٱ�ʶ��!\n");
			break;
		case 4:
			printf("�ٷֺ�!\n");
			break;
		case 5:
			printf("ȱ��(!\n");
			break;
		case 6:
			printf("ȱ��)!\n");
			break;
		case 7: 
			printf("ȱ�ٲ�����!\n");
			break;
        case 8: 
			printf("ȱ�ٶ���\n");
            break;
		case 9:
			printf("�����±����Ϊ����\n");
			break;
		case 11:
			printf("������ͷȱ��{!\n");
			break;
		case 12:
			printf("��������ȱ��}!\n");
			break;
		case 13:
			printf("���һ������������Ӧ����main}!\n");
			break;
		case 14:
			printf("ȱ��]\n");
			break;
		case 15:
			printf("����������䲻�Ϸ�\n");
			break;
		case 16:
			printf("����δ��������ֵ����\n");
			break;
		case 17:
			printf("����û�з���ֵ\n");
			break;
		case 21:
			printf("���ű����!\n");
			break;
		case 22:
			printf("����%s�ظ�����!\n", token1);
			break;
		case 23:
			printf("����δ����!\n");
			break;
		case 24:
			printf("������main���������󣬻������������ַ�\n");
			break;
		case 32:
			printf("�����ظ�����!\n");
			break;
		case 33:
			printf("����δ����\n");
			break;
		case 34:
			printf("call������ı�ʶ��%s���Ǳ�����!\n", token1);
			break;
		case 35:
			printf("read������ı�ʶ�����Ǳ�����!\n");
			break;
		case 36:
			printf("��ֵ������ֵ%s���Ǳ�����!\n", token1);
			break;
		case 37:
			printf("���Ӷ�Ӧ�ı�ʶ�����Ǳ�����!\n");
			break;
		case 40:
			printf("do-while��䲻����\n");
			break;
		case 51:
			printf("switch��ı��ʽ���Ǳ���\n");
			break;
		case 53:
			printf("case�����Ϊ����\n");
			break;
		case 54:
			printf("ȱ��ð��\n");
			break;
		case 55:
			printf("ȱ��break\n");
			break;
		default:
			;
	}
	//������ű������
	printf("�������ű�\n");
	printf("����\t��������   ��������\n");
	outputVarSymbol();
	printf("������\t��ڵ�ַ\n");
	outputFunSymbol();
	printf("������Ҫ���ɵ��ı���ʽ���м�����ļ������֣�����·������");
	scanf("%s", Codeout);
	if ((fpCodeText = fopen(Codeout, "w")) == NULL)
	{
		printf("\n����%s����!\n", Codeout);
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
	printf("������Ҫ���ɵĶ�������ʽ���м�����ļ������֣��ṹ��洢��:");
	scanf("%s", Codeout);
	if ((fpCodeBinary = fopen(Codeout, "wb")) == NULL)
	{
		printf("\n����%s����!\n", Codeout);
		es = 10;
		return es;
	}
	fwrite(codes, sizeof(Code), codesIndex, fpCodeBinary);
	fclose(fpCodeBinary);
	return es;
}

//<program> ��{ fun_declaration }<main_declaration>
//<fun_declaration> �� function ID��(�� �� )��< function_body>
//<main_declaration>��  main��(�� �� )�� < function_body>
int program()
{
	int es = 0, i;

	strcpy(codes[codesIndex++].opt, "BR");     //codes����ĵ�һ��ָ����������ת�Ƶ�main��������ڣ���ڵ�ַ��Ҫ����
	
	fscanf(fpTokenin, "%s %s\n", token, token1);
	es = globeDeclaration_list();
	if (es > 0)
		return es;
	while (strcmp(token, "function") == 0)  //�ж��Ƿ��Ǻ�������
	{
		fscanf(fpTokenin, "%s %s\n", token, token1);
		es = fun_declaration();
		if (es != 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", token, token1);
	}
	if (strcmp(token, "ID"))  //��������������main�����Ķ��壬�����ֵΪ��ID��
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

//<fun_declaration> �� function void|int ID��(�� �� )��< function_body>
int fun_declaration()
{
	int es = 0;
	int cx1, i, j;

	if (strcmp(token, "void") && strcmp(token, "int"))    //����voidҲ����int
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
	offset = 2;//����һ���µĺ�������������Ե�ַ��2��ʼ
	strcpy(nowFun, token1);
	strcpy(codes[codesIndex].opt, "ENTER");//������Ŀ�ʼ
	cx1 = codesIndex++;


	fscanf(fpTokenin, "%s %s\n", token, token1);
	if (strcmp(token, "("))
	{
		es = 5;
		return es;
	}
	fscanf(fpTokenin, "%s %s\n", token, token1);   //��ȡ����
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

//<main_declaration>�� main��(�� �� )�� < function_body>
int main_declaration()
{
	int es = 0;
	int cx1;
	
	insert_funSymbol("main", false);
	strcpy(nowFun, "main");
	writeAddressForFun(nowFun, codesIndex);//�Ѻ��������ڵ�ַ���뺯�����ڷ��ű��еĵ�ַ�ֶ�
	offset = 2;//����һ���µĺ�������������Ե�ַ��2��ʼ
	strcpy(codes[codesIndex].opt, "ENTER");//������Ŀ�ʼ
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

//<function_body>��'{'<declaration_list><statement_list>'}'
int function_body()
{
	int es = 0;

	if (strcmp(token, "{")) //�ж��Ƿ�'{'
	{
		es = 11;
		return es;
	}
	
	//offset = 2;    //����һ���µĺ�������������Ե�ַ��2��ʼ
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = declaration_list();
	if (es>0)
		return es;
	
	es = statement_list();
	if (es>0)
		return es;
	if (strcmp(token, "}")) //�ж��Ƿ�'}'
	{
		es = 12;
		return es;
	}
	strcpy(codes[codesIndex].opt, "RETURN");//������Ľ���
	for (int i = 0; i < returnCxIndex; ++i)   //�������return������תλ��
	{
		codes[returnCx[i]].operand = codesIndex;
	}
	returnCxIndex = 0;
	++codesIndex;
	return es;
}

//<declaration_list>��{<declaration_stat>}
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


int declaration_stat()    //����������ͬʱ��ֵ������������������������
{
	int es = 0;
	char name[30];   //��ű�����
	fscanf(fpTokenin, "%s %s\n", &token, &token1);   
	if (strcmp(token, "ID"))   //���ĵ�һ�����ʱ����Ǳ�ʶ������Ϊ�����������֮ǰһ�������ǣ���int
	{
		es = 3;
		return es;
	}
	strcpy(name, token1);    //���浱ǰ��ʶ������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ",") == 0)   //��ʶ��֮���Ƕ���,˵�������ʶ������ͨ������������ű�֮�������������
	{
		es = insert_varSymbol(nowFun, name, variable, 1);
		if (es > 0)
			return es;
		es = declaration_stat();
		return es;
	}
	else if (strcmp(token, ";") == 0)   //֮���Ƿֺţ�������
	{
		es = insert_varSymbol(nowFun, name, variable, 1);
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "=") == 0)  //֮���ǵȺţ�Ҫ����ͨ������ֵ
	{
		es = insert_varSymbol(nowFun, name, variable, 1); 
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);   //�������һ��
		if (strcmp(token, "NUM") == 0)   //������������ֵ
		{
			strcpy(codes[codesIndex].opt, "LOADI");
			codes[codesIndex++].operand = atoi(token1);
			strcpy(codes[codesIndex].opt, "STO");
			int address;
			es = lookup_varSymbol(name, address, nowFun, variable, 0);
			codes[codesIndex++].operand = address;
		}
		else if(strcmp(token, "ID") == 0)      //������������ֵ
		{
			int address;
			es = lookup_varSymbol(token1, address, nowFun, variable, 0);
			if (es > 0)   //�����Ҳ�������������ȫ�ֱ���
			{
				es = lookup_varSymbol(token1, address, NULL, globe, 0);
				if (es > 0)
					return es;
				strcpy(codes[codesIndex].opt, "LOADA");   //ȫ�ֱ����Ա�����ֵ
				codes[codesIndex++].operand = address;
				strcpy(codes[codesIndex].opt, "STO");
				es = lookup_varSymbol(name, address, nowFun, variable, 0);
				codes[codesIndex++].operand = address;
			}
			else    //��ͨ�����Ա�����ֵ
			{
				strcpy(codes[codesIndex].opt, "LOAD");
				codes[codesIndex++].operand = address;
				strcpy(codes[codesIndex].opt, "STO");
				es = lookup_varSymbol(name, address, nowFun, variable, 0);
				codes[codesIndex++].operand = address;
			}
		}
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if(strcmp(token, ",") == 0)      //֮���Ƕ��ţ���������
			es = declaration_stat();
		if (strcmp(token, ";") == 0)
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "[") == 0)   //���Ǹ�����
	{
		int size = 0;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "NUM"))    //�ն�����������С������Ϊ����
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
		if (strcmp(token, ",") == 0)      //֮���Ƕ��ţ���������
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

//<statement_list>��{<statement>}
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

//<statement>�� <if_stat>|<while_stat>|<for_stat>
//               |<compound_stat> |<expression_stat>| <call _stat>
//int statement()
//{
//	int es = 0;
//	if (es == 0 && strcmp(token, "if") == 0) es = if_stat(); //<if���>
//	if (es == 0 && strcmp(token, "while") == 0) es = while_stat(); //<while���>
//	if (es == 0 && strcmp(token, "for") == 0) es = for_stat(); //<for���>
//	if (es == 0 && strcmp(token, "do") == 0) es = dowhile_stat(); 												   //���ڴ˴����do������
//	if (es == 0 && strcmp(token, "read") == 0) es = read_stat(); //<read���>
//	if (es == 0 && strcmp(token, "write") == 0) es = write_stat(); //<write���>
//	if (es == 0 && strcmp(token, "{") == 0) es = compound_stat(); //<�������>
//	if (es == 0 && strcmp(token, "call") == 0) es = call_stat();//<�����������>
//	if (es == 0 && (strcmp(token, "ID") == 0 || strcmp(token, "NUM") == 0 || strcmp(token, "(") == 0)) es = expression_stat(); //<���ʽ���>
//	if (es == 0 && strcmp(token, "return") == 0) es = return_stat();  //<�����������>
//	if (es == 0 && strcmp(token, "switch") == 0) es = switch_stat();
//	if (es == 0 && strcmp(token, "continue") == 0) es = continue_stat();
//	if (es == 0 && strcmp(token, "break") == 0) es = break_stat();
//	return es;
//}

int statement()
{
	int es = 0;
	if (es == 0 && strcmp(token, "if") == 0) es = if_stat(); //<if���>
	else if (es == 0 && strcmp(token, "while") == 0) es = while_stat(); //<while���>
	else if (es == 0 && strcmp(token, "for") == 0) es = for_stat(); //<for���>
	else if (es == 0 && strcmp(token, "do") == 0) es = dowhile_stat(); 												   //���ڴ˴����do������
	else if (es == 0 && strcmp(token, "read") == 0) es = read_stat(); //<read���>
	else if (es == 0 && strcmp(token, "write") == 0) es = write_stat(); //<write���>
	else if (es == 0 && strcmp(token, "{") == 0) es = compound_stat(); //<�������>
	else if (es == 0 && strcmp(token, "call") == 0) es = call_stat();//<�����������>
	else if (es == 0 && (strcmp(token, "ID") == 0 || strcmp(token, "NUM") == 0 || strcmp(token, "(") == 0)) es = expression_stat(); //<���ʽ���>
	else if (es == 0 && strcmp(token, "return") == 0) es = return_stat();  //<�����������>
	else if (es == 0 && strcmp(token, "switch") == 0) es = switch_stat();
	else if (es == 0 && strcmp(token, "continue") == 0) es = continue_stat();
	else if (es == 0 && strcmp(token, "break") == 0) es = break_stat();
	return es;
}

//<if_stat>�� if '('<expr>')' <statement > [else < statement >]
int if_stat()
{
	int es = 0, cx1, cx2; //if
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))
		return es = 5; //��������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ")"))
		return es = 6; //��������
	strcpy(codes[codesIndex].opt, "BRF");
	cx1 = codesIndex++;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es > 0)
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	cx2 = codesIndex++;
	codes[cx1].operand = codesIndex;
	if (strcmp(token, "else") == 0) //else���ִ���
	{
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		es = statement();
		if (es > 0)
			return es;
	}
	codes[cx2].operand = codesIndex;
	return es;
}

//<while_stat>�� while '('<expr >')' < statement >
int while_stat()
{
	loop = 1;
	int es = 0;
	int cx1, cxEntrance;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  return(es = 5); //��������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	cxEntrance = codesIndex;
	loop_next = cxEntrance;//Ϊ��continue�ص���һ��ѭ����
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ")"))
		return es = 6; //��������
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
		//	����
		codes[loop_end].operand = codesIndex;
		loop_end = 0;

	}
	loop = 0;//��־�˳�ѭ����	
	loop_next = 0;//��־�˳�ѭ��
	return es;
}

//<for_stat>�� for'('<expr>,<expr>,<expr>')'<statement>
int for_stat()
{
	loop = 1;
	int es = 0;
	int cx1, cx2, cxExp2, cxExp3;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  return(es = 5); //��������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	es = expression();

	if (es > 0) 
		return es;
	if (strcmp(token, ";")) 	
		return(es = 4); //�ٷֺ�
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
		return(es = 4); //�ٷֺ�
	cxExp3 = codesIndex;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0) 
		return es;
	strcpy(codes[codesIndex].opt, "BR");
	codes[codesIndex].operand = cxExp2;
	codesIndex++;
	codes[cx2].operand = codesIndex;
	if (strcmp(token, ")"))  return(es = 6); //��������
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
		//	����
		codes[loop_end].operand = codesIndex;
		loop_end = 0;

	}
	loop = 0;//��־�˳�ѭ����
	loop_next = 0;
	return es;
}

//<write_stat>��write <expression>;
int write_stat()
{
	int es = 0;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = expression();
	if (es > 0)
		return es;
	if (strcmp(token, ";"))  
		return (es = 4); //�ٷֺ�
	strcpy(codes[codesIndex++].opt, "OUT");
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//<read_stat>��read ID;
int read_stat()
{
	int es = 0, address;
	char name[30];
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	strcpy(name, token1);   //�������
	if (strcmp(token, "ID"))  
		return(es = 3); //�ٱ�ʶ��
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "[") == 0)   //����
	{
		int index = 0;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		index = atoi(token1);
		if (strcmp(token, "NUM"))  //�±�
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
		es = lookup_varSymbol(name, address, NULL, globe, 0);  //read ȫ�ֱ���
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
		return(es = 4); //�ٷֺ�
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//<compound_stat>��'{'<statement_list>'}'
int compound_stat()    //������亯��
{
	int es = 0;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement_list();
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

//< call _stat>��call ID'(' callparameter_list ')'
int call_stat()
{
	int es = 0, address;
	bool havePara;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	if (strcmp(token, "ID")) 
		return (es = 3); //�ٱ�ʶ��

	es = lookup_funSymbol(token1, address, haveRet, havePara);
	if (es > 0) 
		return es;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "("))  
		return (es = 5); //��(
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = parameter_list();
	if (es > 0) 
		return es;
	if (strcmp(token, ")"))  
		return(es = 6); //��)
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";")) 
		return(es = 4); //�ٷֺ�

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	strcpy(codes[codesIndex].opt, "CAL");
	codes[codesIndex++].operand = address;
	return es;
}

//<expression_stat>��<expression>;|;
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
		return es;//�ٷֺ�
	}
}

//<expression>�� ID=<bool_expr>|<bool_expr>|ID=call_stat()
int expression()       
{
	int es = 0, fileadd;
	bool isGlobe = false;   //����ֵ�ı����Ƿ�Ϊȫ�ֱ���
	bool haveAddress = false;
	char token2[20], token3[40];
	int address;
	if (strcmp(token, "ID") == 0) 
	{
		fileadd = ftell(fpTokenin); //��ס��ǰ�ļ�λ��
		fscanf(fpTokenin, "%s %s\n", &token2, &token3);
		if (strcmp(token2, "[") == 0)   //���Ǹ�����
		{
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);  
			if (strcmp(token2, "NUM"))    //�������±�
			{
				es = 9;
				return es;
			}
			es = lookup_varSymbol(token1, address, nowFun, array, atoi(token3));
			haveAddress = true;    //ȡ������Ԫ�صĵ�ַ
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
		if (strcmp(token2, "=") == 0) //Ҫ�Ա�����ֵ
		{
			if(!haveAddress)   //��������������飬��Ҫȡ��ַ
				es = lookup_varSymbol(token1, address, nowFun, variable, 0);
			if (es > 0)   //��ȫ�ֱ�����ֵ
			{
				es = lookup_varSymbol(token1, address, NULL, globe, 0);
				isGlobe = true;
				if (es > 0)
					return es;
			}
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
			if (strcmp(token, "call") == 0)    //�ú����ķ���ֵ��ֵ
			{
				es = call_expr();
			}
			else
				es = bool_expr();  //����Ҫ����ֵ
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
			fseek(fpTokenin, fileadd, 0); //����'='���ļ�ָ��ص�'='ǰ�ı�ʶ��
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
		strcpy(token2, token);      //���������
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
		if (strcmp(token2, "&&") == 0)   //�롢�򡢷�����
			strcpy(codes[codesIndex++].opt, "AND");
		if (strcmp(token2, "||") == 0)
			strcpy(codes[codesIndex++].opt, "OR");
		if(strcmp(token2, "!") == 0)
			strcpy(codes[codesIndex++].opt, "NOT");
	}
	return es;
}

//<additive_expr>��<term>{(+|-)< term >}
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

//< term >��<factor>{(*| /)< factor >}
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

//< factor >��'('<additive_expr>')'| ID|NUM
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
			return es = 6;    //��������
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
	}
	else    
	{
		char token2[20], token3[30];
		int address;
		if (strcmp(token, "ID") == 0)
		{
			fscanf(fpTokenin, "%s %s\n", &token2, &token3);
			if (strcmp(token2, "[") == 0)   //���Ǹ�����
			{
				fscanf(fpTokenin, "%s %s\n", &token2, &token3);
				if (strcmp(token2, "NUM"))
				{
					es = 9;
					return es;
				}
				int index = atoi(token3);
				es = lookup_varSymbol(token1, address, nowFun, array, index);  //�õ���ַ
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
				es = lookup_varSymbol(token1, address, nowFun, variable, 0); //����ű���ȡ������ַ
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
			es = 7; //ȱ�ٲ�����
			return es;
		}
	}
	return es;
}


//<parameter_list>->{<parameter_stat>}
int parameter_list()
{
	int es = 0;
	if (strcmp(token, ")") == 0)    //û�в���
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
	es = additive_expr();   //�����������ʽ���
	if (es>0) 
		return es;
	if (strcmp(token, ";")) 
		return (es = 4);
	strcpy(codes[codesIndex++].opt, "PUSH");
	strcpy(codes[codesIndex].opt, "BR");    //����return������ת���ĺ���������λ�ã�����������
	returnCx[returnCxIndex++] = codesIndex++;  //��������BRָ�����м�����λ��
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}


int call_expr()   //�������õĺ��������з���ֵ
{
		int es = 0, address;
		bool havePara;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "ID"))
			return (es = 3); //�ٱ�ʶ��
		es = lookup_funSymbol(token1, address, haveRet, havePara);

		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		if (strcmp(token, "("))
			return (es = 5); //��(
		if (havePara)   //�в���
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
		return es = 5; //��������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "ID"))
		return es = 51; //switch�ݶ�Ϊֻ�ܱ���
	strcpy(temp, token1);//��¼����ֵ

	fscanf(fpTokenin, "%s %s\n", &token, &token1);

	if (strcmp(token, ")"))
		return es = 6; //��������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);


	if (strcmp(token, "{"))
		return es = 1; //��������

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "case") == 0)
		es = case_stat(temp);
	if (es>0)
		return es;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "}"))
		return es = 2; //��������
}

//<case_stat>
int case_stat(char * temp)
{
	int es = 0, cx1, cx2, cx_break, address;

	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "NUM"))
		return es = 53;//case���ֻ��Ϊ����

					   //�ж���������͵�ǰcaseֵ�Ƿ�ƥ��
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
	strcpy(codes[codesIndex].opt, "BRF");//�����������ô���������
	cx2 = codesIndex++;//����¼��ǰBRF��λ��
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ":"))
		return es = 54;//case������Ҫð��
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es>0)
		return es;

	if (strcmp(token, "break"))
		return es = 55; //��break
	strcpy(codes[codesIndex].opt, "BR");
	cx_break = codesIndex++;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //�ٷֺ�
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	codes[cx2].operand = codesIndex;
	if (strcmp(token, "case") == 0)
	{
		es = case_stat(temp);
		if (es>0)	
			return es;
		//����case��û�ҵ�������һ��case���ң��ϴ�ƥ��ʧ�ܵ�BRF�е�cx2���ŵ�����Ŀ�ͷ
	}
	else if (strcmp(token, "default") == 0) 
	{
		//���е�case��䶼û���ҵ�����ֻ��ִ������default�����
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
		return es = 54;//default������Ҫð��
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	es = statement();
	if (es>0)
		return es;
	
	if (strcmp(token, "break"))
		return es = 55; //��break
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //�ٷֺ�
	switch_end = codesIndex;
	return es;
}



int break_stat()
{
	int es = 0;
	if (loop == 0)
		return 0;
	strcpy(codes[codesIndex].opt, "BR");
	loop_end = codesIndex++;//��ϣ����ֱ������ĩβ�����ﷵ�صȴ�����
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ";"))
		return es = 4; //�ٷֺ�
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
		return es = 4; //�ٷֺ�
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	return es;
}

int dowhile_stat() 
{
	/*��statement�м�һ���������do�͵�����ľ���*/
	loop = 1;//��־����ѭ����
	int es = 0;
	int cx, cx2;
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	cx = codesIndex;
	es = statement();
	if (es>0)
		return es;
	if (strcmp(token, "while"))
		return (es = 40);//40����do while�����ȱ��while
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
	{      //	����
		codes[loop_end].operand = codesIndex;
		loop_end = 0;
	}
	loop = 0;//��־�˳�ѭ����
	loop_next = 0;//��־�˳�ѭ��
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
	char name[30];   //��ű�����
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, "ID"))   //���ĵ�һ�����ʱ����Ǳ�ʶ������Ϊ�����������֮ǰһ�������ǣ���int
	{
		es = 3;
		return es;
	}
	strcpy(name, token1);    //���浱ǰ��ʶ������
	fscanf(fpTokenin, "%s %s\n", &token, &token1);
	if (strcmp(token, ",") == 0)   //��ʶ��֮���Ƕ���,˵�������ʶ������ͨ������������ű�֮�������������
	{
		es = insert_varSymbol(NULL, name, globe, 1);    //ȫ�ֱ�����һ������ΪNULL
		++globeVar;
		if (es > 0)
			return es;
		es = declaration_stat();
		return es;
	}
	else if (strcmp(token, ";") == 0)   //֮���Ƿֺţ�������
	{
		es = insert_varSymbol(NULL, name, globe, 1);
		++globeVar;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	else if (strcmp(token, "=") == 0)  //֮���ǵȺţ�Ҫ��ȫ�ֱ�����ֵ
	{
		es = insert_varSymbol(NULL, name, globe, 1);
		++globeVar;
		if (es > 0)
			return es;
		fscanf(fpTokenin, "%s %s\n", &token, &token1);   //�������һ��
		if (strcmp(token, "NUM") == 0)   //������������ֵ
		{
			strcpy(codes[codesIndex].opt, "LOADI");
			++len_globeVarInitCX;
			if (startGlobeCode == true)     //����ȫ�ֱ�����ʼ������ĵ�һ������¼λ��
			{
				startGlobeCode = false;
				globeVarInitCX = codesIndex; 
			}
			codes[codesIndex++].operand = atoi(token1);
			strcpy(codes[codesIndex].opt, "STOA");    //ȫ�ֱ�������ָ��
			++len_globeVarInitCX;
			int address;
			es = lookup_varSymbol(name, address, NULL, globe, 0);
			codes[codesIndex++].operand = address;
		}
		else if (strcmp(token, "ID") == 0)      //ȫ�ֱ�����ȫ�ֱ�����ֵ
		{
			int address;
			es = lookup_varSymbol(token1, address, NULL, globe, 0);
			if (es > 0)
				return es;
			strcpy(codes[codesIndex].opt, "LOADA");   //ȫ�ֱ���ȡ��ָ��
			++len_globeVarInitCX;
			if (startGlobeCode == true)     //����ȫ�ֱ�����ʼ������ĵ�һ������¼λ��
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
		if (strcmp(token, ",") == 0)      //֮���Ƕ��ţ���������
			es = declaration_stat();
		if (strcmp(token, ";") == 0)
			fscanf(fpTokenin, "%s %s\n", &token, &token1);
		return es;
	}
	//ȫ�ֱ�������Ϊ����
	else
	{
		es = 15;
		return es;
	}
	return es;
}