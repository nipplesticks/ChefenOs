#include "memblockdevice.h"
#include <stdexcept>

void MemBlockDevice::init()
{
	// Creates index linkedlist
	headNode = buildList();
	
	reset();
}

MemBlockDevice::MemBlockDevice(int nrOfBlocks): BlockDevice(nrOfBlocks) 
{
	init();
}

MemBlockDevice::MemBlockDevice(const MemBlockDevice &other) : BlockDevice(other) 
{
	*this = other;
}

MemBlockDevice::~MemBlockDevice() 
{
    /* Implicit call to base-class destructor */
	// Basicly empties the blocks,
	while (getFreeHDDIndex() != -1);
		
}

MemBlockDevice& MemBlockDevice::operator=(const MemBlockDevice &other) {
    delete [] this->memBlocks;
    this->nrOfBlocks = other.nrOfBlocks;
    this->memBlocks = new Block[this->nrOfBlocks];
	copyList(other.headNode, headNode);
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

/* Resets all the blocks and creates the root directory*/
void MemBlockDevice::reset() {
    for (int i = 0; i < this->nrOfBlocks; ++i) {
        this->memBlocks[i].reset('0');
    }

	char* rootName = constChartoChar("root");
	char* rootType = constChartoChar("/");

	int HDDadress = getFreeHDDIndex();
	// Special case where the root node have its own adress as a parent adress
	Inode root(rootType, rootName, HDDadress, HDDadress);
	
	// Gives the root folder 10 blocks
	for (int i = 0; i < root.getNrOfBlocks() - 2; i++)
	{
		root.setBlock(getFreeHDDIndex());
	}

	char* content = root.toCharArray();
	writeBlock(0, content);
	delete[] content;
}

int MemBlockDevice::size() const {
    return this->nrOfBlocks;
}

int * MemBlockDevice::get10FreeBlockAdresses()
{
	int* blocks = new int[10];
	for (int i = 0; i < 10; i++)
	{
		blocks[i] = getFreeHDDIndex();
	}
	return blocks;
}

char * MemBlockDevice::constChartoChar(const char * string) const
{
	int charSize = 0;
	while (string[charSize++] != '\0');
	char* returnChar = new char[charSize];
	for (int i = 0; i < charSize; i++)
		returnChar[i] = string[i];

	return returnChar;
}

std::string MemBlockDevice::toFile()
{
	std::string content;
	
	content += std::to_string(nrOfBlocks) + "\r\n";
	for (int i = 0; i < nrOfBlocks; i++)
	{
		for (int curblock = 0; curblock < 512; curblock++)
		{
			content += memBlocks[i][curblock];
		}		
	}
	int size = 0;
	int* freeBlocks = getAllFreeBlocks(size);
	for (int i = 0; i < size; i++)
	{
		content += std::to_string(freeBlocks[i]) + " ";
	}

	delete[] freeBlocks;
	return content;
}

Block * MemBlockDevice::getPtrOfBlock(int index)
{
	
	return &this->memBlocks[index];
}

MemBlockDevice::IntNode * MemBlockDevice::buildList(int nrOfBlocks)
{
	IntNode* head = nullptr;

	for (int i = nrOfBlocks; i >= 0; i--)
	{
		IntNode* newNode = new IntNode(head, i);
		head = newNode;
	}
	return head;
}

void MemBlockDevice::createList(int * blocks, int size)
{
	// Clear current list
	while (getFreeHDDIndex() != -1);

	for (int i = size; i >= 0; i--)
	{
		IntNode* newNode = new IntNode(headNode, blocks[i]);
		headNode = newNode;
	}
}

void MemBlockDevice::copyList(IntNode* source, IntNode*& destination)
{
	// Clears the destination node
	IntNode** destPointer = &destination;
	while (*destPointer)
	{
		IntNode* deletedNode = *destPointer;
		*destPointer = deletedNode->m_NextNode;
		deletedNode->m_NextNode = nullptr;
		delete deletedNode;
	}
	
	// Reads all the values from argument head and stores them in array
	int array1[250], counter = 0;
	IntNode** doublePointer = &source;
	while (*doublePointer)
	{
		array1[counter++] = (*doublePointer)->m_Index;
		doublePointer = &(*doublePointer)->m_NextNode;
	}
	// Transfer values backwards into this->headNode
	for (int i = nrOfBlocks; i >= 0; i--)
	{
		IntNode* newNode = new IntNode(destination, array1[i]);
		destination = newNode;
	}
}

int * MemBlockDevice::getAllFreeBlocks(int & size)
{
	IntNode* temp = nullptr;
	copyList(headNode, temp); // Copies the entire list into temp
	
	int* adresses = new int[nrOfBlocks];
	
	int value = 0;
	int i = 0;
	for (; i < nrOfBlocks && value != -1; i++)
	{
		value = getFreeHDDIndex();
		adresses[i] = value;
	}
	size = i-1;
	copyList(temp, headNode);

	IntNode** doublePointer = &temp;
	while (*doublePointer)
	{

		IntNode* deletedNode = *doublePointer;
		*doublePointer = deletedNode->m_NextNode;
		deletedNode->m_NextNode = nullptr;
		delete deletedNode;
	
	}
	return adresses;
}

int MemBlockDevice::getFreeHDDIndex()
{
	IntNode** doublePointer = &headNode;

	if (!*doublePointer)
		return -1;
	
	IntNode* deletedNode = *doublePointer;
	*doublePointer = deletedNode->m_NextNode;
	deletedNode->m_NextNode = nullptr;
	int value = deletedNode->m_Index;
	delete deletedNode;

	return value;
}
