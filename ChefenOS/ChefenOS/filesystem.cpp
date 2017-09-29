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

std::string FileSystem::currentDir() const
{
	return currentInode->getName();
}


/* Please insert your code */