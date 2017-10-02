#include "memblockdevice.h"
#include <stdexcept>

void MemBlockDevice::init()
{
	reset();
	char* lol = new char[5];
	lol[0] = 'r';
	lol[1] = 'o';
	lol[2] = 'o';
	lol[3] = 't';
	lol[4] = '\0';
	Inode root("/", lol, 0, 0);
	// Gives the root folder 12 blocks
	for (int i = 1; i < root.getNrOfBlocks() + 1; i++)
	{
		root.setBlock(i);
	}
	freePointer = root.getNrOfBlocks(); // Next free block
	char* content = root.toCharArray();
	writeBlock(0, content);
	delete[] content;
}

MemBlockDevice::MemBlockDevice(int nrOfBlocks): BlockDevice(nrOfBlocks) 
{

	init();
}

MemBlockDevice::MemBlockDevice(const MemBlockDevice &other) : BlockDevice(other) {

}


MemBlockDevice::~MemBlockDevice() {
    /* Implicit call to base-class destructor */
}

MemBlockDevice& MemBlockDevice::operator=(const MemBlockDevice &other) {
    delete [] this->memBlocks;
    this->nrOfBlocks = other.nrOfBlocks;
    this->freePointer = other.freePointer;
    this->memBlocks = new Block[this->nrOfBlocks];

    for (int i = 0; i < this->nrOfBlocks; ++i)
        this->memBlocks[i] = other.memBlocks[i];

    return *this;
}

Block& MemBlockDevice::operator[](int index) const {
    if (index < 0 || index >= this->nrOfBlocks) {
        throw std::out_of_range("Illegal access\n");
    }
    else {
        return this->memBlocks[index];
    }
}

int MemBlockDevice::spaceLeft() const {
    /* Not yet implemented */
    return 0;
}

int MemBlockDevice::writeBlock(int blockNr, const std::vector<char> &vec) {
    int output = -1;    // Assume blockNr out-of-range

    if (blockNr < this->nrOfBlocks && blockNr >= 0) {
        /* -2 = vec and block dont have same dimensions */
        /* 1 = success */
        output = this->memBlocks[blockNr].writeBlock(vec);
    }
    return output;
}

int MemBlockDevice::writeBlock(int blockNr, const std::string &strBlock) {
    int output = -1;    // Assume blockNr out-of-range

    if (blockNr < this->nrOfBlocks && blockNr >= 0) {
        /* -2 = str-length and block dont have same dimensions */
        /* 1 = success */
        output = this->memBlocks[blockNr].writeBlock(strBlock);
    }
    return output;
}

int MemBlockDevice::writeBlock(int blockNr, const char cArr[]) {
    int output = -1;    // Assume blockNr out-of-range
    if (blockNr < this->nrOfBlocks && blockNr >= 0) {
        output = 1;
        // Underlying function writeBlock cannot check array-dimension.
        this->memBlocks[blockNr].writeBlock(cArr);
    }
    return output;
}

Block MemBlockDevice::readBlock(int blockNr) const {
    if (blockNr < 0 || blockNr >= this->nrOfBlocks)
        throw std::out_of_range("Block out of range");
    else {
        Block a(this->memBlocks[blockNr]);
        return a;
    }
}

/* Resets all the blocks */
void MemBlockDevice::reset() {
    for (int i = 0; i < this->nrOfBlocks; ++i) {
        this->memBlocks[i].reset('0');
    }
	char* lol = new char[5];
	lol[0] = 'r';
	lol[1] = 'o';
	lol[2] = 'o';
	lol[3] = 't';
	lol[4] = '\0';
	Inode root("/", lol, 0, 0);
	// Gives the root folder 12 blocks
	for (int i = 1; i < root.getNrOfBlocks() + 1; i++)
	{
		root.setBlock(i);
	}
	freePointer = root.getNrOfBlocks(); // Next free block
	char* content = root.toCharArray();
	writeBlock(0, content);
	delete[] content;
}

int MemBlockDevice::size() const {
    return this->nrOfBlocks;
}

int * MemBlockDevice::getFreeBlockAdresses() 
{
	int* blocks = new int[12];
	// Gives 12 free blocks
	for (unsigned i = 0; i < 12; i++)
	{
		blocks[i] = freePointer++;
	}
	return blocks;
}

std::string MemBlockDevice::toFile() const
{
	std::string content;
	content += std::to_string(freePointer) + "\r\n";
	content += std::to_string(nrOfBlocks) + "\r\n";
	for (int i = 0; i < nrOfBlocks; i++)
	{
	for (int curblock = 0; curblock < 512; curblock++)
	content += memBlocks[i][curblock];
	}
	return content;
}
