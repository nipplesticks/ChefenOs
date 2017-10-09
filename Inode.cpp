#include "Inode.h"

Inode::Inode(char * type, char * name, int hddLoc, int parentHDDLoc) 
	: type(type), name(name), hddLoc(hddLoc), parentHDDLoc(parentHDDLoc)
{
	timestamp = time(0);
	nrOfBlocks = 12;
	
	this->blockIndexes[0] = this->hddLoc;
	this->usedBlocks[0] = true;
	this->blockIndexes[1] = this->parentHDDLoc;
	this->usedBlocks[1] = true;

	for (int i = 2; i < nrOfBlocks; i++)
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

	data >> hddLoc;
	data >> parentHDDLoc;
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
/* Sets first available block to inputed adress.
Return true if there is available blocks to write*/
bool Inode::setBlock(int adress)
{
	bool wrote = false;
	for (int i = 2; i < nrOfBlocks && !wrote; i++)
	{
		if (blockIndexes[i] == -1)
		{
			blockIndexes[i] = adress;
			wrote = true;
		}
	}

	return wrote;
}

/* Return true if there is a block to lock*/
bool Inode::lockFirstAvailableBlock()
{
	int index = freeBlockInInode();
	if(index != -1)
		usedBlocks[index] = true;
	return index != -1;
}

bool Inode::removeNodeAt(int index)
{
	bool removed = false;
	if (index > 1 && index < nrOfBlocks)
	{
		removed = usedBlocks[index];
		usedBlocks[index] = false;
	}
	

	return removed;
}

/*Converts all variables into a char array*/
char * Inode::toCharArray() const
{
	char *buffert = new char[512];
	
	buffert[511] = '\0';
	int index = 0;
	while (buffert[index] != '\0') buffert[index++] = '0';
	int buffertIndex = 0;


	if (copyCharArrln(buffert, buffertIndex, type) &&
		copyCharArrln(buffert, buffertIndex, name) &&
		copyIntln(buffert, buffertIndex, hddLoc) &&
		copyIntln(buffert, buffertIndex, parentHDDLoc) &&
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

	return buffert;
}
int Inode::freeBlockInInode()
{
	int index = 0;
	bool found = false;
	for (; index < nrOfBlocks && !found;index++)
	{
		if (usedBlocks[index] == false)
			found = true;
			
	}
	return found ? (index - 1) : -1;
}


void Inode::cleanup()
{
	delete[] name;
	name = nullptr;
	delete[] type;
	type = nullptr;

}
void Inode::charDeepCopy(char *src, char*& dst) const
{
	int charLength = 0;
	while (src[charLength] != '\0')
		charLength++;
	dst = new char[charLength + 1];
	int index = 0;
	while (src[index] != '\0')
	{
		dst[index] = src[index];
		index++;
	}
	dst[index] = '\0';
}
void Inode::copy(const Inode & other)
{
	// Yttlig kopiering
	newUsed = true;
	charDeepCopy(other.name, name);
	
	charDeepCopy(other.type, type);
	hddLoc = other.hddLoc;
	parentHDDLoc = other.parentHDDLoc;

	timestamp = other.timestamp;
	nrOfBlocks = other.nrOfBlocks;
	for(int i = 0; i < nrOfBlocks;i++)
		usedBlocks[i] = other.usedBlocks[i];
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

	return buffertIndex < 512;
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

void Inode::setParentHDDLoc(int adress)
{
	this->parentHDDLoc = adress;
}

void Inode::setHDDLoc(int adress)
{
	this->hddLoc = adress;

}

int Inode::getHDDLoc() const
{
	return hddLoc;
}

int Inode::getParentHDDLoc() const
{

	return parentHDDLoc;
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
/* Return true if block is in use */
bool Inode::isBlockUsed(int index) const
{
	return usedBlocks[index];
}

int Inode::getNrOfBlocks() const
{
	return nrOfBlocks;
}

int Inode::getHDDadress(int index) const
{
	if(index != -1)
		return blockIndexes[index];
	return -1;
}
