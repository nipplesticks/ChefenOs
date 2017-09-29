#include "filesystem.h"

FileSystem::FileSystem()
{
	currentInode = new Inode(mMemblockDevice.readBlock(0));
	
}

FileSystem::~FileSystem()
{

}
// WORK IN PROGRESS
void FileSystem::createFolder(char * name)
{
	Inode* newInode = new Inode("Folder", name, 0, currentInode->getID());
	
}

std::string FileSystem::listDir() const
{
	int nrOfBlocks = currentInode->getNrOfBlocks();
	int * blockIndexes = new int[nrOfBlocks];
	std::string lsStr = "";

	for (int i = 0; i < nrOfBlocks; i++)
	{
		blockIndexes = currentInode->getBlockIndex[i];
		if (blockIndexes[i] != NOT_USED)
		{
			Inode printer(mMemblockDevice.readBlock(blockIndexes[i]));
			lsStr += printer.getName() + printer.getType() + "\n";
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
	return currentInode->getName();
}


/* Please insert your code */