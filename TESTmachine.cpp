#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include "var.h"
using namespace std;

map<string, int> Map;
extern char Codeout[30];
extern int globeVar;
extern int globeVarInitCX;
extern int len_globeVarInitCX;
bool haveInitGlobe = true;
int TESTmachine();

Code midCodes[1000];    //�м����
enum enum_Opt {LOAD, LOADI, STO, STI, ADD, SUB,
               MULT, DIV, BR, BRF, EQ, NOTEQ, GT,
               LES, GE, LE, AND, OR, NOT, IN, OUT,
			   RETURN,ENTER,CAL,LOADA,STOA,PUSH,POP};

void init()    //��ʼ��
{
    Map["LOAD"] = LOAD;
    Map["LOADI"] = LOADI;
    Map["STO"] = STO;
    Map["STI"] = STI;
    Map["ADD"] = ADD;
    Map["SUB"] = SUB;
    Map["MULT"] = MULT;
    Map["DIV"] = DIV;
    Map["BR"] = BR;
    Map["BRF"] = BRF;
    Map["EQ"] = EQ;
    Map["NOTEQ"] = NOTEQ;
    Map["GT"] = GT;
    Map["LES"] = LES;
    Map["GE"] = GE;
    Map["LE"] = LE;
    Map["AND"] = AND;
    Map["OR"] = OR;
    Map["NOT"] = NOT;
    Map["IN"] = IN;
    Map["OUT"] = OUT;
    Map["RETURN"] = RETURN;
    Map["ENTER"] = ENTER;
    Map["CAL"] = CAL;
	Map["LOADA"] = LOADA;
	Map["STOA"] = STOA;
	Map["POP"] = POP;
	Map["PUSH"] = PUSH;
}

int TESTmachine()
{

    int es = 0;

    int numCodes; //�м�����ļ���ָ�������
	
    FILE *fin; //����ָ���м�����ļ���ָ��

    int stack[100], top = 0, base=0;   //top��ջ������һ����Ԫ���±�  
	int num[1000];   //���ݶ�ջ���������ͷ���ֵ��
	int num_top = 0;
    int ip=0;//ָ��ָ��
    Code instruction;
	init();
    if((fin = fopen(Codeout, "rb")) == NULL) 
	{
        printf("\n��%s����!\n", Codeout);
        es = 10;
        return(es);
    }

    numCodes = 0;
    while(fread(midCodes + (numCodes++), sizeof(Code), 1, fin) == 1);
    numCodes--;

    top = globeVar + len_globeVarInitCX;   //��stack֮ǰ���ȫ�ֱ�����ȫ�ֱ�����ʼ��Ҫʹ�õĿռ�
    base=top;
    ip = globeVarInitCX;     //��ȫ�ֱ����ĳ�ʼ��ָ�ʼִ��
    stack[top]=0;
    stack[top+1]=0;
	top = globeVar;
    do
	{ //ִ��ÿ��ָ��
		if (ip == globeVarInitCX + len_globeVarInitCX && haveInitGlobe)   //��ʼ��ָ��ִ����ص�0�Ŵ���
		{
			haveInitGlobe = false;
			top = base;
			ip = 0;
		}
        instruction=midCodes[ip++];
		switch (Map[instruction.opt])
		{
		case LOAD://LOAD D��D�е����ݼ��ص�������ջ��
			stack[top] = stack[base + instruction.operand];
			top++;
			break;

		case LOADI://LOADI a������aѹ�������ջ
			stack[top] = instruction.operand;
			top++;
			break;

		case STO:
			top--;
			stack[base + instruction.operand] = stack[top];
			break;

		case ADD://ADD����ջ����Ԫ��ջ����Ԫ���ݳ�ջ����ӣ�������ջ����
			top--;
			stack[top - 1] = stack[top - 1] + stack[top];
			break;

		case SUB://SUB    ����ջ����Ԫ��ȥջ����Ԫ���ݲ���ջ��������ջ����
			top--;
			stack[top - 1] = stack[top - 1] - stack[top];
			break;

		case MULT://MULT   ����ջ����Ԫ��ջ����Ԫ���ݳ�ջ����ˣ�������ջ����
			top--;
			stack[top - 1] = stack[top - 1] * stack[top];
			break;

		case DIV://DIV    ����ջ����Ԫ��ջ����Ԫ���ݳ�ջ�������������ջ����
			top--;
			stack[top - 1] = stack[top - 1] / stack[top];
			break;

		case BR://BR    lab  ������ת�Ƶ�lab
			ip = instruction.operand;
			break;

		case BRF://BRF  lab  ���ջ����Ԫ�߼�ֵ����Ϊ�٣�0����ת�Ƶ�lab
			top--;
			if (stack[top] == 0)
				ip = instruction.operand;
			break;

		case EQ://EQ  ��ջ������Ԫ�����ڱȽϣ�����������٣�1��0������ջ��
			top--;
			stack[top - 1] = (stack[top - 1] == stack[top]);
			break;

		case NOTEQ://NOTEQ ��ջ������Ԫ�������ڱȽϣ�����������٣�1��0������ջ��
			top--;
			stack[top - 1] = (stack[top - 1] != stack[top]);
			break;

		case GT://GT    ��ջ������ջ������������ջ����1��������0
			top--;
			stack[top - 1] = stack[top - 1] > stack[top];
			break;

		case LES://LES  ��ջ��С��ջ������������ջ����1��������0
			top--;
			stack[top - 1] = stack[top - 1] < stack[top];
			break;

		case GE://GE  ��ջ�����ڵ���ջ������������ջ����1��������0
			top--;
			stack[top - 1] = stack[top - 1] >= stack[top];
			break;

		case LE://LE  ��ջ��С�ڵ���ջ������������ջ����1��������0
			top--;
			stack[top - 1] = stack[top - 1] <= stack[top];
			break;

		case AND://AND ��ջ������Ԫ���߼������㣬����������٣�1��0������ջ��
			top--;
			stack[top - 1] = stack[top - 1] && stack[top];
			break;

		case OR://OR  ��ջ������Ԫ���߼������㣬����������٣�1��0������ջ��
			top--;
			stack[top - 1] = stack[top - 1] || stack[top];
			break;

		case NOT://NOT  ��ջ�����߼�ֵȡ��
			stack[top - 1] = !stack[top - 1];
			break;

		case IN://IN �ӱ�׼�����豸�����̣�����һ���������ݣ�����ջ��
			printf("���������ݣ�");
			scanf("%d", &stack[top]);
			top++;
			break;

		case OUT://OUT ��ջ����Ԫ���ݳ�ջ�����������׼����豸�ϣ���ʾ������
			top--;
			printf("�������:%d\n", stack[top]);
			break;

		case ENTER://���뺯��
			top += instruction.operand; //��ջ��Ϊ������������������
			break;

		case RETURN:
			top = base;//�ͷű���������ջ�еĿռ�
			ip = stack[top + 1]; //ȡ�����������еķ��ص�ַ
			base = stack[top]; //�ָ����������Ļ���ַ
			break;

		case CAL:
			stack[top] = base; //���������Ļ���ַ��ջ
			stack[top + 1] = ip; //���������ķ��ص�ַ��ջ��ip��CALָ�����м���������е��±�
			base = top; // �ı����ַΪ���������Ļ���ַ
			ip = instruction.operand; //ת�뱻�������ĺ����壬��һ��ִ�б��������ĵ�һ��ָ��λ��
			break;

		case LOADA:    
			stack[top] = stack[instruction.operand];
			top++;
			break;

		case STOA:
			top--;
			stack[instruction.operand] = stack[top];
			break;

		case POP:
			stack[top] = num[num_top - 1];
			top++;
			num_top--;
			break;

		case PUSH:
			num[num_top] = stack[top - 1];
			num_top++;
			break;
		}
    }while(ip !=0);
    return(es);
}






