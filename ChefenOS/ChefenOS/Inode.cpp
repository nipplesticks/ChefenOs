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
#include <string>
Inode::Inode(const Block & block)
{
	std::stringstream data;
	std::string StringConverter;
	data << block.toString();

	data >> StringConverter;
	int size = StringConverter.size() + 1;
	type = new char[size];
	for(int i = 0; i < StringConverter.size(); i++)
		 type[i] = StringConverter[i];
	type[size - 1] = '\0';

	data >> StringConverter;
	size = StringConverter.size() + 1;
	name = new char[size];
	for (int i = 0; i < StringConverter.size(); i++)
		name[i] = StringConverter[i];
	name[size - 1] = '\0';;

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


	if (copyCharArrln(buffert, buffertIndex, type) &&
		copyCharArrln(buffert, buffertIndex, name) &&
		copyIntln(buffert, buffertIndex, id) &&
		copyIntln(buffert, buffertIndex, up) &&
		copyIntln(buffert, buffertIndex, timestamp) &&
		copyIntln(buffert, buffertIndex, nrOfBlocks) &&
		copyIntln(buffert, buffertIndex, usedBlocks))
	{
		for (int i = 0; i < nrOfBlocks; i++)
		{
			
			copyIntln(buffert, buffertIndex, blockIndexes[i]);

		}
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
// WORK IN PROGRESS
bool Inode::connectInode(const Inode & inode)
{
	int index = 0;
	for (; index < nrOfBlocks; index++)
		if (blockIndexes[index] != NOT_USED)
			break;
		else
			return false;


	
	return false;
}

void Inode::cleanup()
{
	//delete name;
	//delete type;
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
/* Copy char* into char[] buffer*/
bool Inode::copyCharArrln(char buffert[], int & buffertIndex, const char * arr) const
{
	int index = 0;

	while (arr[index] != '\0' && buffertIndex < 511)
	{
		buffert[buffertIndex++] = arr[index++];
	}
	buffert[buffertIndex++] = '\r';
	buffert[buffertIndex++] = '\n';

	return buffertIndex < 511;
}
/* Copy integer to char[] buffer THis has to be fixed cross-platformed*/
bool Inode::copyIntln(char buffert[], int & buffertIndex, int value) const
{
	std::string str = std::to_string(value);
	const char * integerBuffer = str.c_str();

	int index = 0;
	while (integerBuffer[index] != '\0')
	{
		buffert[buffertIndex++] = integerBuffer[index++];
	}

	buffert[buffertIndex++] = '\r';
	buffert[buffertIndex++] = '\n';

	return buffertIndex < 511;
	/*
	char integerBuffer[65];

	_itoa_s(value, integerBuffer, 65, 10);

	int index = 0;
	while (integerBuffer[index] != '\0')
		buffert[buffertIndex++] = integerBuffer[index++];

	buffert[buffertIndex++] = '\r';
	buffert[buffertIndex++] = '\n';
	return buffertIndex < 511;
	*/
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

int Inode::getID() const
{
	return id;
}

time_t Inode::getTimeStamp() const
{
	return timestamp;
}

const char * Inode::getName() const
{
	return name;
}

int Inode::getUsedBlocks() const
{
	return usedBlocks;
}

int Inode::getNrOfBlocks() const
{
	return nrOfBlocks;
}

int Inode::getBlockIndex(int index) const
{
	return blockIndexes[index];
}
