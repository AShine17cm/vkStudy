#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

struct MediaAsset
{
public:
	virtual ~MediaAsset() = default;
};
struct Song:public MediaAsset
{
public:
	std::wstring artist;
	std::wstring title;

	Song(const std::wstring& artist_,const std::wstring& title_) :artist{ artist_ }, title{ title_ }
	{	}
};

struct Photo:public MediaAsset
{
public:
	std::wstring date;
	std::wstring location;
	std::wstring subject;

	Photo(const std::wstring& date_,const std::wstring& location_,const std::wstring& subject_)
		:date{ date_ }, location{ location_ }, subject{ subject_ }
	{	}
};

int main() {
	auto sp1 = std::make_shared<Song>( L"Mm01", L"Loved01" );	//推荐方法
	std::shared_ptr<Song> sp1_x = std::shared_ptr<Song>(new Song(L"Mm010", L"Loved010"));
	std::shared_ptr<Song> sp2(new Song(L"Mm02", L"Loved02"));

	std::shared_ptr<Song> sp5(nullptr);
	std::shared_ptr<Song> sp5_x;
	sp5 = std::make_shared<Song>(L"Mm05", L"Loved050");

	
	auto sp3(sp2);			//通过copy constructor  ,ref+1
	auto sp4 = sp2;			//通过assignment ,		ref+1

	std::shared_ptr<Song> sp7(nullptr);

	sp1.swap(sp2);			//交换引用 和 计数

	std::vector<std::shared_ptr<Song>> v{
		std::make_shared<Song>(L"Bob Dylan", L"The Times They Are A Changing"),
		std::make_shared<Song>(L"Aretha Franklin", L"Bridge Over Troubled Water"),
		std::make_shared<Song>(L"Thalía", L"Entre El Mar y Una Estrella")
	};
	std::vector<std::shared_ptr<Song>> v2;
	
	std::remove_copy_if(v.begin(), v.end(), std::back_inserter(v2), 
		[](std::shared_ptr<Song> s) 
		{
			return s->artist.compare(L"Bob Dylan") == 0;
		});

	for (const auto& s : v2) 
	{
		std::wcout << s->artist << L":" << s->title << std::endl;
	}

	return 0;
}