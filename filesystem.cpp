#include "filesystem.h"

FileSystem::FileSystem()
{
	currentInode = new Inode(mMemblockDevice.readBlock(0));
}

FileSystem::~FileSystem()
{

}


/* Please insert your code */