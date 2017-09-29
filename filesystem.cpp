#include "filesystem.h"

FileSystem::FileSystem()
{
	currentInode = new Inode(mMemblockDevice.readBlock(0));
	
}

FileSystem::~FileSystem()
{
	delete currentInode;
}
/* Creates a folder entry in the current INode*/
void FileSystem::createFolder(char * folderName)
{
	if (isNameUnique(folderName))
	{
		int hddWriteIndex = currentInode->getBlockIndex(currentInode->freeBlockInInode());
		Inode* newInode = new Inode("/", folderName, hddWriteIndex, currentInode->getHDDLoc());

		int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
		for (int i = 0; i < newInode->getNrOfBlocks(); i++)
			newInode->setBlock(freeBlocks[i]);
		delete freeBlocks;

		if (currentInode->lockFirstAvailableBlock())
		{
			mMemblockDevice.writeBlock(currentInode->getHDDLoc(), currentInode->toCharArray());
			mMemblockDevice.writeBlock(newInode->getHDDLoc(), newInode->toCharArray());

		}
	}
	
}
/* Lists all available entires in current INode*/
std::string FileSystem::listDir() const
{
	int nrOfBlocks = currentInode->getNrOfBlocks();
	int * blockIndexes = new int[nrOfBlocks];
	std::string lsStr = "";

	for (int i = 1; i < nrOfBlocks; i++)
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
/* Return current I-Node name+type*/
std::string FileSystem::currentDir() const
{
	std::string dir = currentInode->getName();
	dir += currentInode->getType();
	return dir;
}
bool FileSystem::changeDir(char * folderPath)
{
	Inode * walker = new Inode(*currentInode);
	Inode* temp = walkDir(walker, folderPath);
	if (temp != nullptr)
	{
		delete currentInode;
		currentInode = temp;
	}


	return temp != nullptr;
}
/* Compares all the names in the current Inode
Return false if name found */
bool FileSystem::isNameUnique(char * name)
{
	//Read available blocks and store names
	int numberOfBlocks = currentInode->getNrOfBlocks();
	std::string* names = new std::string[numberOfBlocks];
	for(int i = 0; i < numberOfBlocks; i++)
		if (currentInode->ifUsedBlock(i))
		{
			Block currentBlock = mMemblockDevice.readBlock(currentInode->getBlockIndex(i));
			Inode curInode(currentBlock);
			names[i] = curInode.getName();
			if (name == names[i])
				return false;

		}
	delete[] names;
	return true;
}

Inode * FileSystem::walkDir(Inode* currentDirectory, char * next)
{
	int index = 0;
	int walker = 0;
	bool foundSlash = false;
	char inThisFolder[25];
	char toNextFolder[4096];

	//find current Folder
	while (next[index] != '\0' && !foundSlash)
	{
		if (next[index] != '/')
		{
			inThisFolder[walker++] = next[index++];
		}
		else
		{
			foundSlash = true;
		}
	}
	
	inThisFolder[walker] = '\0';
	if (next[index] != '\0')
	{
		index++;
	}
	walker = 0;
	while (next[index] != '\0')
	{
		toNextFolder[walker++] = next[index++];
	}
	toNextFolder[walker] = '\0';
	//Hitta mappen i current directory
	Block currentBlock;
	if (!strcmp(inThisFolder, ".."))
	{
		currentBlock = mMemblockDevice.readBlock(currentDirectory->getBlockIndex(0));
		Inode * temp = new Inode(currentBlock);
		delete currentDirectory;
		currentDirectory = new Inode(*temp);
		delete temp;
		return walkDir(currentDirectory, toNextFolder);
	}
	else
	{
		for (int i = 0; i < currentDirectory->getNrOfBlocks() && inThisFolder[0] != '\0'; i++)
		{
			if (currentDirectory->ifUsedBlock(i))
			{
				Block currentBlock = mMemblockDevice.readBlock(currentDirectory->getBlockIndex(i));

				Inode * temp = new Inode(currentBlock);
				if (!strcmp(temp->getName(), inThisFolder))
				{
					delete currentDirectory;
					currentDirectory = new Inode(*temp);
					delete temp;
					return walkDir(currentDirectory, toNextFolder);
				}

			}

		}
	}
	
	

	if (next[0] == '\0')
	{
		return currentDirectory;
	}

	return nullptr;
	//Läser från disk och hämtar mappens Inode
	//Kalla funktionen igen fast med nya Inode pekaren och 
	// resterande path. 


	return nullptr;
}


/* Please insert your code */