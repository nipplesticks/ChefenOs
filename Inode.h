#ifndef Inode_H
#define Inode_H
#include <ctime>
#include "block.h"
#include <sstream>

#define NOT_USED -1

class Inode
{
private:
	//Metadata
	
	char* type;
	char* name;
	int hddLoc;
	int parentHDDLoc;
	time_t timestamp;
	int nrOfBlocks;
	bool usedBlocks[12];
	int blockIndexes[12];

	bool newUsed = false;

public:
	Inode(char * type, char * name, int hddLoc, int parentHDDLoc);
	Inode(const Block & block);
	Inode(const Inode &other);

	~Inode();

	void setName(char *&name);
	int getHDDLoc() const;
	int getParentHDDLoc() const;
	time_t getTimeStamp() const;
	const char* getType() const;
	const char* getName() const;

	/* Return true if block is in use */
	bool isBlockUsed(int index)  const;
	int getNrOfBlocks() const;
	int getBlockIndex(int index) const;

	bool setBlock(int adress);

	char* toCharArray() const;
	
	/* Retun free block within the Inode*/
	int freeBlockInInode();
	
	Inode& operator=(const Inode &other);
	bool lockFirstAvailableBlock();

private:
	void cleanup();
	void copy(const Inode &other);
	bool copyCharArrln(char buffert[], int &buffertIndex, const char * arr) const ;
	bool copyIntln(char buffert[], int &buffertIndex, int value) const;


};

#endif