#include <conio.h>
#include <iostream>

/* ���̵������¼� */
int mainx2() 
{
	while (true)
	{
		if (_kbhit()) 
		{
			int ch = _getch();
			switch (ch)
			{
			case 'a':
				std::cout << "char is:" << (int)'a' << "   " << ch << std::endl;
				break;
			case 'w':
				std::cout << "char is:" << ch- 'w'<< std::endl;
				break;
			}
		}
	}
	return 0;
}