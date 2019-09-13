# complier
简单的C语言编译器（编译原理课程作业）
仅支持int类型数据，支持for, while, if, case, do-while流程控制语句
使用递归下降法进行语法分析，生成中间代码在虚拟机上解释执行。
var.h——使用的数据结构定义
scan.cpp——词法分析程序
parse.cpp——语法分析和语义分析程序，生成中间代码
TESTmachine.cpp——虚拟机
