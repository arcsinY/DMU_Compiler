#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <algorithm>
#include "var.h"
#define keywordSum 17
extern char scanIn[300], scanOut[300];
extern FILE *fin, *fout;

err all[50];
char *keyword[keywordSum] = { "if","else","for","while","do","int","read","write","call","function","void","return","break","switch","case","default","continue"};
char singleWord[50] = "+-*(){};,:[]";
char doubleWord[10] = "<>=!&|";
int line = 1;
int count = 0;
int errorIndex = 0;

bool searchKeyword(char* s);
bool cmp(char* a, char* b);
void showError();

void TESTscan()
{
	char ch, token[40];
	int es = 0, j;
	std::sort(keyword, keyword + keywordSum, cmp);
	printf("������Դ�ļ��ļ�����");
	scanf("%s", scanIn);
	printf("�����뵥�����ļ��ļ�����");
	scanf("%s", scanOut);
	if ((fin = fopen(scanIn, "r")) == NULL)
	{
		all[errorIndex].es = 1;
		++count;
		all[errorIndex++].lineNum = line;
		return;
	}
	if ((fout = fopen(scanOut, "w")) == NULL)
	{
		all[errorIndex].es = 2;
		++count;
		all[errorIndex++].lineNum = line;
		return;
	}
	ch = getc(fin);
	while (ch != EOF)
	{
		while (ch == ' ' || ch == '\t' || ch == '\n')      //���Կո�
		{
			if (ch == '\n')
				++line;
			ch = getc(fin);
		}
		if (isalpha(ch))   //����ĸ
		{
			token[0] = ch;
			j = 1;
			ch = getc(fin);
			while (isalnum(ch))    //һֱ����ĸ������
			{
				token[j++] = ch;
				ch = getc(fin);
			}
			token[j] = '\0';
			if (!searchKeyword(token))          //�ó�һ�����ʣ����ǲ��ǹؼ���
				fprintf(fout, "%-10s%-10s\n", "ID", token);
			else
				fprintf(fout, "%-10s%-10s\n", token, token);
		}
		else if (isdigit(ch))  //���ֿ�ͷ��һ���ǳ���
		{
			token[0] = ch;
			j = 1;
			ch = getc(fin);
			while (isdigit(ch)) //һֱ������
			{
				token[j++] = ch;
				ch = getc(fin);
			}
			token[j] = '\0';
			fprintf(fout, "%-10s%-10s\n", "NUM", token);
		}
		else if (strchr(singleWord, ch) > 0)         //���ָ�����ͷ�����ǵ��ָ���
		{
			token[0] = ch;
			token[1] = '\0';
			ch = getc(fin);
			fprintf(fout, "%-10s%-10s\n", token, token);
		}
		else if (strchr(doubleWord, ch) > 0)     //˫�ָ�����ͷ
		{
			token[0] = ch;       
			if (ch == '&')     //��ͷ&��֮��Ҳ������&
			{
				ch = getc(fin);
				if (ch == '&')
				{
					token[1] = '&';
					token[2] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
					ch = getc(fin);
				}
				else
				{
					token[1] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
					all[errorIndex].es = 4;
					++count;
					all[errorIndex++].lineNum = line;
				}
			}
			else if (ch == '|')  //��ͷ|��֮��һ��ҲҪ��|
			{
				ch = getc(fin);
				if (ch == '|')
				{
					token[1] = '|';
					token[2] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
					ch = getc(fin);
				}
				else
				{
					token[1] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
					all[errorIndex].es = 5;
					++count;
					all[errorIndex++].lineNum = line;
				}
			}
			else if (ch == '>' || ch == '<' || ch == '=' || ch == '!')    //��ͷ����Щ���ţ�Ҫ�ж�֮��һ���ǲ���=
			{
				token[0] = ch;
				ch = getc(fin);
				if (ch == '=')       //��=������˫�ָ���
				{
					token[1] = ch;
					token[2] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
					ch = getc(fin);
				}
				else
				{
					token[1] = '\0';
					fprintf(fout, "%-10s%-10s\n", token, token);
				}
			}
		}
		else if (ch == '/')     //��/��������ע�Ͳ��֣������ǳ���
		{
			ch = getc(fin);
			if (ch == '*')     //֮����*���ⲿ����ע��
			{
				char ch1;
				ch1 = getc(fin);
				do
				{
					ch = ch1;
					ch1 = getc(fin);
				} while ((ch != '*' || ch1 != '/') && ch1 != EOF);
				ch = getc(fin);
			}
			else    //���Ǹ�����
			{
				token[0] = '/';
				token[1] = '\0';
				fprintf(fout, "%-10s%-10s\n", token, token);
			}
		}	
		else
		{
			token[0] = ch;
			token[1] = '\0';
			ch = getc(fin);
			all[errorIndex].es = 3;
			++count;
			all[errorIndex++].lineNum = line;
			fprintf(fout, "%-10s%-10s\n", token, token);
		}
	}
	fclose(fin);
	fclose(fout);
	showError();
	return;
}


bool cmp(char *a, char *b)
{
	if (strcmp(a, b) < 0)
		return true;
	else
		return false;
}

void showError()
{
	if (count == 0)
		printf("\n\n�ʷ��������\n");
	else
	{
		printf("\n\n�ʷ���������%d������!\n", count);
		for (int i = 0; i < count; i++)
		{
			switch (all[i].es)
			{
			case 1:
				printf("Դ�ļ���ʧ��\n");
				break;
			case 2:
				printf("����������ļ���ʧ��\n");
				break;
			case 3:
				printf("��%d���ִ������޷�ʶ����ַ�\n", all[i].lineNum);
				break;
			case 4:
				printf("��%d���ִ���ȱ��&\n", all[i].lineNum);
				break;
			case 5:
				printf("��%d���ִ���ȱ��|\n", all[i].lineNum);
				break;
			dafault:
				;
			}
		}
	}
}

bool searchKeyword(char* s) 
{
	int high = keywordSum - 1;
	int low = 0;
	int mid;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (strcmp(keyword[mid], s) == 0)
			return true;
		else if (strcmp(keyword[mid], s) < 0)
			low = mid + 1;
		else
			high = mid - 1;
	}
	return false;
}
