#include "filesystem.h"

FileSystem::FileSystem()
{
	// Initilize root directory
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
	std::string *folderNames = seperateSlashes(folderName, arraySize);
	Inode* currentHolder = currentInode;
	while (arrayIndex != arraySize)
	{	
		if (isNameUnique(folderNames[arrayIndex].c_str()))
		{
			int hddWriteIndex = currentInode->getBlockIndex(currentInode->freeBlockInInode());

			char* fmita = stringToCharP(folderNames[arrayIndex]);
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

/* Fully working changeDir method with support for multiple slashes */
bool FileSystem::changeDir2(char * folderPath)
{
	// Initialize variables
	int nrOfInodeBlocks = currentInode->getNrOfBlocks();
	int stringSize = 0, stringIndex = 0;
	std::string* folder = seperateSlashes(folderPath, stringSize);
	Inode* tempNode = currentInode;
	//Locate folder in current Inode table
	for (int i = 0; i < nrOfInodeBlocks && !(stringSize == stringIndex); i++)
	{
		if (folder[stringIndex] == "..")
		{
			Block curBlock = mMemblockDevice.readBlock(tempNode->getParentHDDLoc());
			tempNode = new Inode(curBlock);
			
			//Resets and check next path
			i = 0;
			stringIndex++;

			//changeCurrentInode(tempNode);

		}else if (tempNode->isBlockUsed(i))
		{
			Block curBlock = mMemblockDevice.readBlock(tempNode->getBlockIndex(i));
			Inode* tempNode2 = new Inode(curBlock);

			//Checking if the folder is in the currentInode table
			if (!strcmp(tempNode2->getName(), folder[stringIndex].c_str()))
			{
				//Change workingDirectory
				//changeCurrentInode(tempNode);

				//Resets and check next path
				i = 0;
				stringIndex++;
				tempNode = tempNode2;
				
			}
			else
			{ 
				delete tempNode2;
				tempNode2 = nullptr;
			}
		}

	}
	delete[] folder;
	if (stringIndex == stringSize) changeCurrentInode(tempNode);
	return stringIndex == stringSize;
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
	// Calculates the amount of char* in between the /:s
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
	int dirIndex = 0, charIndex = 0;
	while (dirIndex < size)
	{
		char currentChar = filepath[charIndex++];

		if (currentChar != '\0' && currentChar != '/') 
			directories[dirIndex] += currentChar;
		else // Reached a '/'
			dirIndex++;
	}
	return directories;
}

char * FileSystem::stringToCharP(const std::string& string) const
{
	char* newChar = new char[string.length()];
	strncpy_s(newChar, string.length() + 1, string.c_str(), string.length() + 1);
	newChar[string.length()] = '\0';
	return newChar;
}

void FileSystem::changeCurrentInode(Inode * newCur)
{
	delete currentInode;
	currentInode = newCur;

	// Updates name
	currentDirectory = currentInode->getName();
	currentDirectory += currentInode->getType();
}


/* Please insert your code */