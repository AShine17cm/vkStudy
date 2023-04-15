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
	MyData stackData{ 1314 };	//ջ������

	MyData* pData = new MyData( 43 );	//��������
	//std::unique_ptr<MyData> pData2 = std::unique_ptr<MyData>(new MyData( 322));//ջ��ָ��+��������
	std::unique_ptr<MyData> pData2 (new MyData(322));//ջ��ָ��+��������
	std::unique_ptr<MyData> pData3(new MyData(324));
	std::cout << pData2->val << std::endl;
	int val;
	std::cin >> val;
	std::cout << "val:" << val << std::endl;
	//pData3.release();	//��ǰ�ͷ� ְ��ָ�� 324
	pData3.reset();		//��ǰ�ͷ� ����ָ�� 324
	delete pData;	//�ͷ�  ��������  43
	//... �ͷ� ջ��ָ��+��������  322
	//... �ͷ� ջ������ 1314

	return 0;
}