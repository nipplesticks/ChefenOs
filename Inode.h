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
	int id;
	int up;
	time_t timestamp;
	int nrOfBlocks;
	int usedBlocks;
	int blockIndexes[12];

	Inode& operator=(const Inode &other);
public:
	Inode(char * type, char * name, int id, int up);
	Inode(const Block & block);
	Inode(const Inode &other);

	~Inode();

	void setName(char *&name);
	int getID() const;
	time_t getTimeStamp() const;
	const char* getName() const;
	int getUsedBlocks() const;
	int getNrOfBlocks() const;
	int getBlockIndex(int index) const;

	bool setBlock(int adress);

	char* toBytes() const;

	bool connectInode(const Inode& inode);

private:
	void cleanup();
	void copy(const Inode &other);
	bool copyCharArrln(char buffert[], int &buffertIndex, const char * arr) const ;
	bool copyIntln(char buffert[], int &buffertIndex, int value) const;


};

#endif