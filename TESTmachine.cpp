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

Code midCodes[1000];    //中间代码
enum enum_Opt {LOAD, LOADI, STO, STI, ADD, SUB,
               MULT, DIV, BR, BRF, EQ, NOTEQ, GT,
               LES, GE, LE, AND, OR, NOT, IN, OUT,
			   RETURN,ENTER,CAL,LOADA,STOA,PUSH,POP};

void init()    //初始化
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

    int numCodes; //中间代码文件中指令的条数
	
    FILE *fin; //用于指向中间代码文件的指针

    int stack[100], top = 0, base=0;   //top：栈顶的下一个单元的下标  
	int num[1000];   //数据堆栈，传参数和返回值用
	int num_top = 0;
    int ip=0;//指令指针
    Code instruction;
	init();
    if((fin = fopen(Codeout, "rb")) == NULL) 
	{
        printf("\n打开%s错误!\n", Codeout);
        es = 10;
        return(es);
    }

    numCodes = 0;
    while(fread(midCodes + (numCodes++), sizeof(Code), 1, fin) == 1);
    numCodes--;

    top = globeVar + len_globeVarInitCX;   //在stack之前存放全局变量和全局变量初始化要使用的空间
    base=top;
    ip = globeVarInitCX;     //从全局变量的初始化指令开始执行
    stack[top]=0;
    stack[top+1]=0;
	top = globeVar;
    do
	{ //执行每条指令
		if (ip == globeVarInitCX + len_globeVarInitCX && haveInitGlobe)   //初始化指令执行完回到0号代码
		{
			haveInitGlobe = false;
			top = base;
			ip = 0;
		}
        instruction=midCodes[ip++];
		switch (Map[instruction.opt])
		{
		case LOAD://LOAD D将D中的内容加载到操作数栈。
			stack[top] = stack[base + instruction.operand];
			top++;
			break;

		case LOADI://LOADI a将常量a压入操作数栈
			stack[top] = instruction.operand;
			top++;
			break;

		case STO:
			top--;
			stack[base + instruction.operand] = stack[top];
			break;

		case ADD://ADD将次栈顶单元与栈顶单元内容出栈并相加，和置于栈顶。
			top--;
			stack[top - 1] = stack[top - 1] + stack[top];
			break;

		case SUB://SUB    将次栈顶单元减去栈顶单元内容并出栈，差置于栈顶。
			top--;
			stack[top - 1] = stack[top - 1] - stack[top];
			break;

		case MULT://MULT   将次栈顶单元与栈顶单元内容出栈并相乘，积置于栈顶。
			top--;
			stack[top - 1] = stack[top - 1] * stack[top];
			break;

		case DIV://DIV    将次栈顶单元与栈顶单元内容出栈并相除，商置于栈顶。
			top--;
			stack[top - 1] = stack[top - 1] / stack[top];
			break;

		case BR://BR    lab  无条件转移到lab
			ip = instruction.operand;
			break;

		case BRF://BRF  lab  检查栈顶单元逻辑值，若为假（0）则转移到lab
			top--;
			if (stack[top] == 0)
				ip = instruction.operand;
			break;

		case EQ://EQ  将栈顶两单元做等于比较，并将结果真或假（1或0）置于栈顶
			top--;
			stack[top - 1] = (stack[top - 1] == stack[top]);
			break;

		case NOTEQ://NOTEQ 将栈顶两单元做不等于比较，并将结果真或假（1或0）置于栈顶
			top--;
			stack[top - 1] = (stack[top - 1] != stack[top]);
			break;

		case GT://GT    次栈顶大于栈顶操作数，则栈顶置1，否则置0
			top--;
			stack[top - 1] = stack[top - 1] > stack[top];
			break;

		case LES://LES  次栈顶小于栈顶操作数，则栈顶置1，否则置0
			top--;
			stack[top - 1] = stack[top - 1] < stack[top];
			break;

		case GE://GE  次栈顶大于等于栈顶操作数，则栈顶置1，否则置0
			top--;
			stack[top - 1] = stack[top - 1] >= stack[top];
			break;

		case LE://LE  次栈顶小于等于栈顶操作数，则栈顶置1，否则置0
			top--;
			stack[top - 1] = stack[top - 1] <= stack[top];
			break;

		case AND://AND 将栈顶两单元做逻辑与运算，并将结果真或假（1或0）置于栈顶
			top--;
			stack[top - 1] = stack[top - 1] && stack[top];
			break;

		case OR://OR  将栈顶两单元做逻辑或运算，并将结果真或假（1或0）置于栈顶
			top--;
			stack[top - 1] = stack[top - 1] || stack[top];
			break;

		case NOT://NOT  将栈顶的逻辑值取反
			stack[top - 1] = !stack[top - 1];
			break;

		case IN://IN 从标准输入设备（键盘）读入一个整型数据，并入栈。
			printf("请输入数据：");
			scanf("%d", &stack[top]);
			top++;
			break;

		case OUT://OUT 将栈顶单元内容出栈，并输出到标准输出设备上（显示器）。
			top--;
			printf("程序输出:%d\n", stack[top]);
			break;

		case ENTER://进入函数
			top += instruction.operand; //在栈中为被调函数开辟数据区
			break;

		case RETURN:
			top = base;//释放被调函数在栈中的空间
			ip = stack[top + 1]; //取得主调函数中的返回地址
			base = stack[top]; //恢复主调函数的基地址
			break;

		case CAL:
			stack[top] = base; //主调函数的基地址入栈
			stack[top + 1] = ip; //主调函数的返回地址入栈，ip是CAL指令在中间代码数组中的下标
			base = top; // 改变基地址为被调函数的基地址
			ip = instruction.operand; //转入被调函数的函数体，下一次执行被调函数的第一条指令位置
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






