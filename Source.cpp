#include "filesystem.h"

int main()
{
	FileSystem gg;
	char* path = "users/hejsan/tjena/";
	int size = 0;
	std::string* dir = gg.seperateSlashes(path, size);

	for (int i = 0; i < size; i++)
		std::cout << dir[i] << std::endl;

	system("pause");
	return 0;

}