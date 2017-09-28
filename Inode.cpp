#include "Inode.h"

Inode::Inode(char * type, char * name, int id, int up) : type(type), name(name), id(id), up(up)
{
	timestamp = time(0);
	nrOfBlocks = 12;
	usedBlocks = 0;
	
	for (int i = 0; i < nrOfBlocks; i++)
	{
		blockIndexes[i] = NOT_USED;
	}



}

Inode::Inode(const Block & block)
{
	std::stringstream data;
	data << block.toString();

	data >> type;
	data >> name;
	data >> id;
	data >> up;
	data >> timestamp;
	data >> nrOfBlocks;
	data >> usedBlocks;
	for (int i = 0; i < nrOfBlocks; i++)
	{
		data >> blockIndexes[i];
	}
}

Inode::Inode(const Inode & other)
{
	copy(other);
}

Inode::~Inode()
{
	cleanup();
}

bool Inode::setBlock(int adress)
{
	bool inRange = usedBlocks != nrOfBlocks;
	if (inRange)
		blockIndexes[usedBlocks++] = adress;
	return inRange;
}

char * Inode::toBytes() const
{
	char *buffert = new char[512];
	int buffertIndex = 0;
	
	const char idc = id + '0';
	const char upc = up + '0';
	const char tsc = timestamp + '0';
	const char nrc = nrOfBlocks + '0';
	const char ubc = usedBlocks + '0';

	if (copyCharArrln(buffert, buffertIndex, type) &&
		copyCharArrln(buffert, buffertIndex, name) &&
		copyCharArrln(buffert, buffertIndex, &idc) &&
		copyCharArrln(buffert, buffertIndex, &upc) &&
		copyCharArrln(buffert, buffertIndex, &tsc) &&
		copyCharArrln(buffert, buffertIndex, &nrc) &&
		copyCharArrln(buffert, buffertIndex, &ubc))
	{
		for (int i = 0; i < nrOfBlocks &&
			copyCharArrln(buffert, buffertIndex, (char*)blockIndexes[i]); i++);
	}

	if (buffertIndex < 511)
	{
		buffert[buffertIndex++] = '\0';
	}
	else
	{
		return nullptr;
	}

	return buffert;
}

void Inode::cleanup()
{
	delete name;
	delete type;
}

void Inode::copy(const Inode & other)
{
	// Yttlig kopiering
	name = other.name;
	type = other.type;
	id = other.id;
	up = other.up;

	timestamp = other.timestamp;
	usedBlocks = other.usedBlocks;
	nrOfBlocks = other.nrOfBlocks;
	for (int i = 0; i < nrOfBlocks; i++)
	{
		blockIndexes[i] = other.blockIndexes[i];
	}
	
}

bool Inode::copyCharArrln(char buffert[], int & buffertIndex, const char * arr) const
{
	int index = 0;

	while (arr[index] != '\0' && buffertIndex < 511)
	{
		buffert[buffertIndex++] = type[index++];
	}
	buffert[buffertIndex++] = '\r';
	buffert[buffertIndex++] = '\n';

	return buffertIndex < 511;
}

Inode & Inode::operator=(const Inode & other)
{
	if (this != &other)
	{
		cleanup();
		copy(other);
	}

	return *this;
}

void Inode::setName(char *& name)
{
	this->name = name;
}
