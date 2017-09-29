#include "filesystem.h"

FileSystem::FileSystem()
{
	currentInode = new Inode(mMemblockDevice.readBlock(0));
	
}

FileSystem::~FileSystem()
{

}
// WORK IN PROGRESS
void FileSystem::createFolder(char * folderName)
{
	int writeIndex = currentInode->getBlockIndex(currentInode->freeBlockInInode());
	Inode* newInode = new Inode("/", folderName, writeIndex, currentInode->getHDDLoc());

	int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
	for (int i = 0; i < newInode->getNrOfBlocks(); i++)
		newInode->setBlock(freeBlocks[i]);
	delete freeBlocks;


	currentInode->writeBlock();
	mMemblockDevice.writeBlock(currentInode->getHDDLoc(), currentInode->toBytes());
	mMemblockDevice.writeBlock(newInode->getHDDLoc(), newInode->toBytes());
	// Skapa noden klart
	// Ge den blocks
	// Sätta in noden i en ledig plats i currentNode
	// Skriver till disk i den platsen
	// Skriver currentNode till disken 
}

std::string FileSystem::listDir() const
{
	int nrOfBlocks = currentInode->getNrOfBlocks();
	int * blockIndexes = new int[nrOfBlocks];
	std::string lsStr = "";

	for (int i = 0; i < nrOfBlocks; i++)
	{
		blockIndexes[i] = currentInode->getBlockIndex(i);
		if (currentInode->ifUsedBlock(i))
		{
			Inode printer(mMemblockDevice.readBlock(blockIndexes[i]));
			lsStr += printer.getName();
			lsStr += printer.getType();
			lsStr += "\n";
		}
		else
		{
			blockIndexes[i] = NOT_USED;
		}
	}

	

	return lsStr;
}

std::string FileSystem::currentDir() const
{
	std::string dir = currentInode->getName();
	dir += currentInode->getType();
	return dir;
}


/* Please insert your code */