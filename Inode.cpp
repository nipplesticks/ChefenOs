#include "Inode.h"

Inode::Inode(char * type, char * name, int id, int InodeBlockAdress) 
	: type(type), name(name), id(id), InodeBlockAdress(InodeBlockAdress)
{
	timestamp = time(0);
	nrOfBlocks = 12;
	
	for (int i = 0; i < nrOfBlocks ; i++)
	{
		blockIndexes[i] = NOT_USED;
		usedBlocks[i] = false;
	}



}

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
	data >> InodeBlockAdress;
	data >> timestamp;
	data >> nrOfBlocks;
	for (int i = 0; i < nrOfBlocks; i++)
		data >> usedBlocks[i];
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
	int index = freeBlockInInode();
	if (index != -1)
	{
		blockIndexes[index] = adress;
	}
	return index != -1;
}

bool Inode::writeBlock()
{
	int index = freeBlockInInode();
	if(index != -1)
		usedBlocks[index] = true;
	return index != -1;
}

char * Inode::toBytes() const
{
	char *buffert = new char[512];
	int buffertIndex = 0;


	if (copyCharArrln(buffert, buffertIndex, type) &&
		copyCharArrln(buffert, buffertIndex, name) &&
		copyIntln(buffert, buffertIndex, id) &&
		copyIntln(buffert, buffertIndex, InodeBlockAdress) &&
		copyIntln(buffert, buffertIndex, timestamp) &&
		copyIntln(buffert, buffertIndex, nrOfBlocks))
	{
		for (int i = 0; i < nrOfBlocks; i++)
		{
			copyIntln(buffert, buffertIndex, usedBlocks[i]);
		}
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
int Inode::freeBlockInInode()
{
	int index = 0;
	for (; index < nrOfBlocks; index++)
		if (usedBlocks[index] == false)
			break;
		else
			return -1;
	return index;
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
	InodeBlockAdress = other.InodeBlockAdress;

	timestamp = other.timestamp;
	for(int i = 0; i < nrOfBlocks;i++)
		usedBlocks[i] = other.usedBlocks[i];
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

	int index = 0;
	while (str[index] != '\0')
	{
		buffert[buffertIndex++] = str[index++];
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

int Inode::getInodeBlockAdress() const
{

	return InodeBlockAdress;
}

time_t Inode::getTimeStamp() const
{
	return timestamp;
}

const char * Inode::getType() const
{
	return type;
}

const char * Inode::getName() const
{
	return name;
}

bool Inode::ifUsedBlock(int index) const
{
	return usedBlocks[index];
}

int Inode::getNrOfBlocks() const
{
	return nrOfBlocks;
}

int Inode::getBlockIndex(int index) const
{
	return blockIndexes[index];
}
