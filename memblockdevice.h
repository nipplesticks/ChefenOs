#ifndef MEMBLOCKDEVICE_H
#define MEMBLOCKDEVICE_H
#include "blockdevice.h"
#include "Inode.h"

class MemBlockDevice: public BlockDevice
{
private:
	void init();
	
public:
    MemBlockDevice(int nrOfBlocks = 250);
	MemBlockDevice(int nrOfBlocks, int freePointer);
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
	/* Return freeblockadresses*/
	int* getFreeBlockAdresses();
	
	void setFreePointer(int freePointer);
	char* constChartoChar(const char * string) const;
	std::string toFile() const;

	Block* getPtrOfBlock(int index);
};

#endif // MEMBLOCKDEVICE_H
