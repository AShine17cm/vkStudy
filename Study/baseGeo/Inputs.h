#pragma once
#include <iostream>
#include <conio.h>		//键盘输入事件
/* 存储输入 */
struct  Inputs
{
	/* ASCII码 */
	int key; 

	void tryGet() 
	{
		if(_kbhit()) 
		{
			key = _getch();
		}
		else
		{
			key = -1;
		}
	}
};


