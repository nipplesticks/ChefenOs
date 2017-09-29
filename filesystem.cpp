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
	Inode* newInode = new Inode("Folder", folderName, 0, currentInode->getInodeBlockAdress());
	int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
	for (int i = 0; i < newInode->getNrOfBlocks(); i++)
		newInode->setBlock(freeBlocks[i]);
	delete freeBlocks;
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