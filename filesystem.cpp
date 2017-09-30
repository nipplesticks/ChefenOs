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
bool FileSystem::createFile(char * fileName, char* content, int sizeInBytes)
{
	
	return false;
}
/* Creates a folder entry in the current INode, supports multiple slashes*/
bool FileSystem::createFolder(char * folderName)
{
	int arraySize = 0, arrayIndex = 0;
	std::string *folderNames = nullptr;
	Inode* currentHolder = nullptr;
	bool done = false;

	// Relative or absolute path?
	if (folderName[0] == '/')
	{
		Block rootBlock = mMemblockDevice.readBlock(0);
		currentHolder = new Inode(rootBlock);
		folderNames = seperateSlashes(++folderName, arraySize);
	}
	else
	{
		currentHolder = new Inode(*currentInode);
		folderNames = seperateSlashes(folderName, arraySize);
	}
	Inode** pointerHolder = new Inode*[arraySize];

	while (arrayIndex != arraySize)
	{	
		if (isNameUnique(folderNames[arrayIndex].c_str(), currentHolder))
		{
			int hddWriteIndex = currentHolder->getBlockIndex(currentHolder->freeBlockInInode());

			char* currentFolder = stringToCharP(folderNames[arrayIndex]);
			Inode *newInode = new Inode("/", currentFolder, hddWriteIndex, currentHolder->getHDDLoc());
			int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
			for (int i = 0; i < newInode->getNrOfBlocks(); i++)
				newInode->setBlock(freeBlocks[i]);
			delete[] freeBlocks;

			if (currentHolder->lockFirstAvailableBlock())
			{
				char* currenINodeContent = currentHolder->toCharArray();
				char* newINodeContent = newInode->toCharArray();

				mMemblockDevice.writeBlock(currentHolder->getHDDLoc(), currenINodeContent);
				mMemblockDevice.writeBlock(newInode->getHDDLoc(), newINodeContent);

				delete[] currenINodeContent;
				delete[] newINodeContent;

			}
			pointerHolder[arrayIndex] = newInode;
			delete currentHolder;
			currentHolder = newInode;
			arrayIndex++;
		}
		else if (arrayIndex != arraySize)
		{

		}
		else
		{
			done = true;
			break;
		}

	}
	delete[] folderNames;
	/*for (int i = 0; i < arraySize; i++)
		delete pointerHolder[i];*/
	delete[] pointerHolder;
	delete currentHolder;
	// Update currentInode 
	Block currentBlock = mMemblockDevice.readBlock(currentInode->getHDDLoc());

	delete currentInode;
	currentInode = new Inode(currentBlock);
	
	return done ? false : true;
	
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
	delete[] blockIndexes;
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
	std::string* folder = nullptr;
	Inode* tempNode = nullptr;

	//Relative or absolute path?
	if (folderPath[0] == '/') // Absolute path
	{
		Block rootBlock = mMemblockDevice.readBlock(0);
		tempNode = new Inode(rootBlock);
		folder = seperateSlashes(++folderPath, stringSize);
	}
	else // Relative path
	{
		tempNode = new Inode(*currentInode);
		folder = seperateSlashes(folderPath, stringSize);
	}

	//Locate folder in current Inode table
	for (int i = 0; i < nrOfInodeBlocks && (stringSize != stringIndex); i++)
	{
		if (folder[stringIndex] == "..")
		{
			Block curBlock = mMemblockDevice.readBlock(tempNode->getParentHDDLoc());
			Inode* tempNode2 = new Inode(curBlock);
			
			delete tempNode;
			tempNode = tempNode2;
			
			//Resets and check next folder
			i = 0;
			stringIndex++;

		}
		else if (tempNode->isBlockUsed(i))
		{
			Block curBlock = mMemblockDevice.readBlock(tempNode->getBlockIndex(i));
			Inode* tempNode2 = new Inode(curBlock);

			//Checking if the folder is in the currentInode table
			if (!strcmp(tempNode2->getName(), folder[stringIndex].c_str()))
			{
				//Resets and check next folder
				i = 0;
				stringIndex++;
				
				delete tempNode;
				tempNode = nullptr;
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
	bool readFullpath = stringIndex == stringSize;
	// Did we read the fullpath?
	if (readFullpath) changeCurrentInode(tempNode); // if fullpath was read then we reached the correct folder
	else delete tempNode;
	return readFullpath;

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
bool FileSystem::isNameUnique(const char * name, const Inode* inode) const
{
	//Read available blocks and store names
	int numberOfBlocks = inode->getNrOfBlocks();
	std::string* names = new std::string[numberOfBlocks];
	for(int i = 0; i < numberOfBlocks; i++)
		if (inode->isBlockUsed(i))
		{
			Block currentBlock = mMemblockDevice.readBlock(inode->getBlockIndex(i));
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
	//L�ser fr�n disk och h�mtar mappens Inode
	//Kalla funktionen igen fast med nya Inode pekaren och 
	// resterande path. 


	return nullptr;
}

std::string * FileSystem::seperateSlashes(char * filepath, int & size) const
{
	size = 0; int index = 0;
	bool wordFound = false;
	while (filepath[index] != '\0')
	{
		if (filepath[index++] != '/')
		{
			wordFound = true;;
		}
		else
		{
			if (wordFound)
			{
				size++;
				wordFound = false;
			}
		}
	}

	if (wordFound) size++;
	
	index = 0;
	std::string *directories = new std::string[size];
	int charIndex = 0, wordIndex = 0;
	while(filepath[index] != '\0')
	{
		if (filepath[index] != '/')
		{
			directories[wordIndex] += filepath[index];
			charIndex++;
		}
		else
		{
			if (charIndex > 0)
			{
				wordIndex++;
			}
		}
		index++;

	
	}
	return directories;
}

char * FileSystem::stringToCharP(const std::string& string) const
{
	char* newChar = new char[string.length()+1];
	strncpy_s(newChar, string.length()+1, string.c_str(), string.length()+1);
	newChar[string.length()] = '\0';
	return newChar;
}

void FileSystem::changeCurrentInode(Inode * newCur)
{
	delete currentInode;
	currentInode = new Inode(*newCur);
	delete newCur;

	// Updates name
	currentDirectory = currentInode->getName();
	currentDirectory += currentInode->getType();
}


/* Please insert your code */