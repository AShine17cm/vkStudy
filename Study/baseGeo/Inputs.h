#pragma once
#include <iostream>
#include <conio.h>		//���������¼�
/* �洢���� */
struct  Inputs
{
	/* ASCII�� */
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


