/*Created by:
Henrik Nilsson the Malaysia airlines bug creator
Magnus Nyqvist the princess of Malaysia airlines
*/
#ifndef MEMBLOCKDEVICE_H
#define MEMBLOCKDEVICE_H
#include "blockdevice.h"
#include "Inode.h"

class MemBlockDevice: public BlockDevice
{
private:
	void init();
	class IntNode
	{
	public:
		IntNode* m_NextNode;
		int m_Index;
		IntNode(IntNode* nextNode = 0, int index = 0)
		{
			m_NextNode = nextNode;
			m_Index = index;
		}
	};
	IntNode* headNode;
public:
    MemBlockDevice(int nrOfBlocks = 250);
    MemBlockDevice(const MemBlockDevice &other);

    ~MemBlockDevice();

    /* Operators */
    MemBlockDevice& operator=(const MemBlockDevice &other);
    Block &operator[] (int index) const;

    /* Returns amount of free blocks */
    int spaceLeft() const;

    /* Writes a block */
    int writeBlock(int blockNr, const std::vector<char> &vec);
    int writeBlock(int blockNr, const std::string &strBlock);
    int writeBlock(int blockNr, const char cArr[]);     // Use with care, make sure that cArr has at least the same dimension as block

    /* Reads a block */
    Block readBlock(int blockNr) const;

    /* Resets all the blocks */
    void reset();

    /* Return the size */
    int size() const;

	/* Return freeblockadresses - linkedList */
	int* get10FreeBlockAdresses();

	char* constChartoChar(const char * string) const;
	std::string toFile();

	Block* getPtrOfBlock(int index);

	bool insertBlockIndex(int index);
	IntNode* buildList(int nrOfBlocks = 250);
	void createList(int* blocks, int size);
	void copyList(IntNode * source, IntNode*& destination);
	
	int* getAllFreeBlocks(int& size);
	int getFreeHDDIndex();
};

#endif // MEMBLOCKDEVICE_H
