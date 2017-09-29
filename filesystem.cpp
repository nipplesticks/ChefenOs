#include "filesystem.h"

FileSystem::FileSystem()
{
	
	currentInode = new Inode(mMemblockDevice.readBlock(0));
	std::string path = currentInode->getName();
	path += currentInode->getType();
	currentDirectory = path;
}

FileSystem::~FileSystem()
{
	delete currentInode;
}
/* Creates a folder entry in the current INode, supports multiple slashes*/
void FileSystem::createFolder(char * folderName)
{
	int arraySize = 0;
	int arrayIndex = 0;
	std::string *folderPath = seperateSlashes(folderName, arraySize);
	Inode* currentHolder = currentInode;
	while (arrayIndex != arraySize)
	{
		char* fmita = new char[folderPath[arrayIndex].length()];
		strncpy_s(fmita, folderPath[arrayIndex].length() + 1, folderPath[arrayIndex].c_str(), folderPath[arrayIndex].length() + 1);
		fmita[folderPath[arrayIndex].length()] = '\0';
		if (isNameUnique(folderPath[arrayIndex].c_str()))
		{
			int hddWriteIndex = currentInode->getBlockIndex(currentInode->freeBlockInInode());
			Inode* newInode = new Inode("/", fmita, hddWriteIndex, currentInode->getHDDLoc());

			int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
			for (int i = 0; i < newInode->getNrOfBlocks(); i++)
				newInode->setBlock(freeBlocks[i]);
			delete freeBlocks;

			if (currentInode->lockFirstAvailableBlock())
			{
				mMemblockDevice.writeBlock(currentInode->getHDDLoc(), currentInode->toCharArray());
				mMemblockDevice.writeBlock(newInode->getHDDLoc(), newInode->toCharArray());

			}

			currentInode = newInode;
			arrayIndex++;
		}

	}

	currentInode = currentHolder;
	
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
		if (currentInode->isBlockUsed(i))
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
	return currentDirectory;
}
void FileSystem::setCurrentDirStr(const std::string & str, bool remove)
{
	//if (remove)
		//currentDirectory.erase(currentDirectory.find(str),currentDirectory.length());
	//else
		currentDirectory = str;
}

/* Think this will become easier one day, NOT FINISHED and NOT IN USE ATM */
bool FileSystem::changeDir2(char * folderPath)
{
	// Initialize variables
	int nrOfInodeBlocks = currentInode->getNrOfBlocks();
	int stringSize = 0, stringIndex = 0;
	std::string* folder = seperateSlashes(folderPath, stringSize);
	std::string* InodeNames = new std::string[nrOfInodeBlocks];
	bool foundDirectory = false;
	//Locate folder in current Inode table
	for (int i = 0; i < nrOfInodeBlocks && !foundDirectory; i++)
	{
		if (currentInode->isBlockUsed(i))
		{
			Block curBlock = mMemblockDevice.readBlock(currentInode->getBlockIndex(i));
			Inode *tempNode = new Inode(curBlock);

			//Checking if the folder is in the currentInode table
			if (!strcmp(tempNode->getName(), folder[stringIndex].c_str()))
			{
				//Unload workingDirectory
				delete currentInode;
				currentInode = tempNode;
				
				//Resets and check next path
				i = 0;
				stringIndex++;

				// Updates name
				currentDirectory = currentInode->getName();

				//Last directory in filepath, if true then done
				if (stringIndex == stringSize)	foundDirectory = true;
				
			}
			else
				delete tempNode;
		}

	}

	return foundDirectory;
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
bool FileSystem::isNameUnique(const char * name)
{
	//Read available blocks and store names
	int numberOfBlocks = currentInode->getNrOfBlocks();
	std::string* names = new std::string[numberOfBlocks];
	for(int i = 0; i < numberOfBlocks; i++)
		if (currentInode->isBlockUsed(i))
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
		std::string path = currentDirectory->getName();
		path += currentDirectory->getType();
		setCurrentDirStr(path, true);
		return walkDir(currentDirectory, toNextFolder);
	}
	else
	{
		for (int i = 0; i < currentDirectory->getNrOfBlocks() && inThisFolder[0] != '\0'; i++)
		{
			if (currentDirectory->isBlockUsed(i))
			{
				Block currentBlock = mMemblockDevice.readBlock(currentDirectory->getBlockIndex(i));

				Inode * temp = new Inode(currentBlock);
				if (!strcmp(temp->getName(), inThisFolder))
				{
					delete currentDirectory;
					currentDirectory = new Inode(*temp);
					delete temp;
					std::string path = currentDirectory->getName();
					path += currentDirectory->getType();
					setCurrentDirStr(path, false);
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

std::string * FileSystem::seperateSlashes(char * filepath, int & size) const
{
	// Calculates the length of char array
	size = 1;
	int index = 0;
	while (filepath[index] != '\0')
	{
		if(filepath[index] == '/')
			size++;
		index++;
	}
	//Place each directoy into seperate index
	std::string* directories = new std::string[size];
	int dirIndex = 0;
	int charIndex = 0;
	int charIn = 0;
	while (dirIndex < size)
	{
		char currentChar = filepath[charIndex++];
		if (currentChar != '\0' && currentChar != '/')
			directories[dirIndex] += currentChar;
		else
		{
			dirIndex++;
			charIn = 0;
		}
	}
	return directories;
}


/* Please insert your code */