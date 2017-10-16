/*Created by:
Henrik Nilsson the Malaysia airlines bug creator
Magnus Nyqvist the princess of Malaysia airlines
*/
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
	int dataSize;
	bool permissions[2];

public:
	Inode(char * type, char * name, int hddLoc, int parentHDDLoc);
	Inode(const Block & block);
	Inode(const Inode &other);

	~Inode();

	void setName(char *name);
	void setParentHDDLoc(int adress);
	void setHDDLoc(int adress);
	
	/*Returns old adress*/
	int changeBlockAdress(int index, int newAdress);

	int getHDDLoc() const;
	int getParentHDDLoc() const;
	time_t getTimeStamp() const;
	const char* getType() const;
	const char* getName() const;
	int getDataSize() const;
	bool getRead() const;
	bool getWrite() const;

	/* Return true if block is in use */
	bool isBlockUsed(int index)  const;
	/* Retrun number of blocks */
	int getNrOfBlocks() const;
	/* Return number of blocks not in use */
	int getNrOfFreeBlocks() const;	
	int getNrOfInUseBlocks() const;
	/* Returns adress to the hdd*/
	int getHDDadress(int index) const;

	bool setBlock(int adress);
	void setDataSize(int size);
	void setPermRead(bool read);
	void setPermWrite(bool write);
	/* Returns all the information from this node to a char*
	   REMEMBER TO DELETE */
	char* toCharArray() const;
	std::string getPermAsString() const;
	
	/* Retun free block within the Inode*/
	int freeBlockInInode();
	
	bool lockFirstAvailableBlock();
	/* Locks block at index */
	bool lockBlockAt(int index);
	/* Unlocks block at index */
	bool unlockBlockAt(int index);

	Inode& operator=(const Inode &other);
private:
	void cleanup();
	void copy(const Inode &other);
	bool copyCharArrln(char buffert[], int &buffertIndex, const char * arr) const ;
	bool copyIntln(char buffert[], int &buffertIndex, int value) const;
	void charDeepCopy(char *src, char*& dst) const;


};

#endif