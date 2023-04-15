#include <iostream>
#include <memory>

struct  MyData
{

	int val;
	MyData(int ofVal) { val = ofVal; }
	~MyData() 
	{
		std::cout << "Destruct of MyData:" << val << std::endl;
	}
};

int mainx3() 
{
	MyData stackData{ 1314 };	//栈上数据

	MyData* pData = new MyData( 43 );	//堆上数据
	//std::unique_ptr<MyData> pData2 = std::unique_ptr<MyData>(new MyData( 322));//栈上指针+堆上数据
	std::unique_ptr<MyData> pData2 (new MyData(322));//栈上指针+堆上数据
	std::unique_ptr<MyData> pData3(new MyData(324));
	std::cout << pData2->val << std::endl;
	int val;
	std::cin >> val;
	std::cout << "val:" << val << std::endl;
	//pData3.release();	//提前释放 职能指针 324
	pData3.reset();		//提前释放 智能指针 324
	delete pData;	//释放  堆上数据  43
	//... 释放 栈上指针+堆上数据  322
	//... 释放 栈上数据 1314

	return 0;
}