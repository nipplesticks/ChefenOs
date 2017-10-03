#include "filesystem.h"

FileSystem::FileSystem()
{
	init();
}

FileSystem::~FileSystem()
{
	delete currentInode;
}
void FileSystem::formatHDD()
{
	delete currentInode;
	mMemblockDevice.reset();
	init();

}
bool FileSystem::createFile(char * fileName, char* content, int sizeInBytes)
{
	
	return false;
}
#include <iostream>
/* Creates a folder entry in the current INode
- Supports multiple slashes
- Supports relative or absoulte path
- */
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

	while (arrayIndex != arraySize)
	{	
		// Does there exist a folder with this name in the current directory?
		if (isNameUnique(folderNames[arrayIndex].c_str(), currentHolder))	
		{
			int hddWriteIndex = currentHolder->getBlockIndex(currentHolder->freeBlockInInode());
			if(hddWriteIndex != -1)
			{
		 
				char* currentFolder = stringToCharP(folderNames[arrayIndex]);
				char* currentType = stringToCharP(std::string("/"));
				Inode *newInode = new Inode(currentType, currentFolder, hddWriteIndex, currentHolder->getHDDLoc());
				int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
				for (int i = 1; i < newInode->getNrOfBlocks(); i++)
					newInode->setBlock(freeBlocks[i]);
				delete[] freeBlocks;

				if (currentHolder->lockFirstAvailableBlock())
				{
					char* currenINodeContent = currentHolder->toCharArray();
					char* newINodeContent = newInode->toCharArray();
//					std::cout << currenINodeContent;
//					std::cout << newInode->getHDDLoc();
					mMemblockDevice.writeBlock(currentHolder->getHDDLoc(), currenINodeContent);
					mMemblockDevice.writeBlock(newInode->getHDDLoc(), newINodeContent);

					delete[] currenINodeContent;
					delete[] newINodeContent;

				}
			//	std::cout << lol();
				delete currentHolder;
				currentHolder = newInode;
				arrayIndex++;
			}/* If it is not the destination folder yet, we will instead traversel into the folder and continue.
				This enables the option to create a folder when you are not inside it */
			else if (arrayIndex != arraySize - 1)	
			{
				char* fName = stringToCharP(folderNames[arrayIndex].c_str());
				Inode* tempCurrent = currentInode;
				currentInode = currentHolder;
				Inode* returnInode = walkDir(fName);
				delete currentHolder;
				currentHolder = new Inode(*returnInode);
				delete returnInode;
				delete[] fName; 
				currentInode = tempCurrent;
				tempCurrent = nullptr;
				arrayIndex++;
			} //If the name is not unique, and we are at the final destination we are done
			else
			{
				done = true;
				break;
			}
			}
	}
	delete[] folderNames;
	delete currentHolder;
	currentHolder = nullptr;
	// Update currentInode 
	refreshCurrentInode();
	return !done;
	
}

void FileSystem::createImage(char * folderPath)
{
	auto file = std::fstream(folderPath, std::ios::out | std::ios::binary);
	if (file.good())
	{
		std::string content = mMemblockDevice.toFile();
		file.write(content.c_str(), content.size());
		file.close();
	}
	
}
bool FileSystem::readImage(char * folderPath)
{
	auto file = std::fstream(folderPath, std::ios::in | std::ios::binary);
	if (file.good())
	{
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();

		file.seekg(0, std::ios_base::beg);

		char* buffer = new char[size+1];
		buffer[size] = '\0';
		file.read(buffer, size);
		
		file.close();
		int bufferIndex = 0;
		std::string container;
		while (buffer[bufferIndex] != '\r')
		{
			container += buffer[bufferIndex++];
		}
		int freePointer = std::stoi(container);
		container = "";
		bufferIndex+=2;
		while (buffer[bufferIndex] != '\r')
		{
			container += buffer[bufferIndex++];
		}
		bufferIndex += 2;
		int nrOfBlocks = std::stoi(container);
		mMemblockDevice.reset();

		
		return true;
	}
	return false;
}

std::string FileSystem::lol()
{
	return mMemblockDevice.toFile();
}
/* Lists all available entires in current INode*/
bool FileSystem::listCopy(char* filepath, std::string& holder)
{
	Inode* tempNode = nullptr;

	if (filepath)
	{
		tempNode = walkDir(filepath);
		if (!tempNode)
		{
			delete tempNode;
			return false;
		}
	}
	else
		tempNode = new Inode(*currentInode);

	int nrOfBlocks = currentInode->getNrOfBlocks();
	int * blockIndexes = new int[nrOfBlocks];

	for (int i = 2; i < nrOfBlocks; i++)
	{
		blockIndexes[i] = tempNode->getBlockIndex(i);
		if (tempNode->isBlockUsed(i))
		{
			Inode printer(mMemblockDevice.readBlock(blockIndexes[i]));
			holder += printer.getName();
			holder += printer.getType();
			holder += "\n";
		}
		else
		{
			blockIndexes[i] = NOT_USED;
		}
	}
	delete[] blockIndexes;
	delete tempNode;
	return true;
}

std::string FileSystem::pwd()
{
	return dirNameJumper(currentInode->getHDDLoc());
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
Inode* FileSystem::walkDir(char * folderPath)
{
	// Initialize variables
	int nrOfInodeBlocks = currentInode->getNrOfBlocks();
	int stringSize = 0, stringIndex = 0;
	std::string* folder = nullptr;
	Inode* tempNode = nullptr;
	// BUG WITH ../
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
	for (int i = 1; i < nrOfInodeBlocks && (stringSize != stringIndex); i++)
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
	//// Did we read the fullpath?
	if (readFullpath) return tempNode;

	delete tempNode;
	return nullptr;

}

std::string FileSystem::dirNameJumper(int index)
{
	Block currentBlock = mMemblockDevice.readBlock(index);
	Inode name(currentBlock);
	if (index == 0) return "/";
	else
		return dirNameJumper(name.getParentHDDLoc()) + name.getName() + name.getType();
}

void FileSystem::init()
{
	// Initilize root directory
	currentInode = new Inode(mMemblockDevice.readBlock(0));

	std::string path = currentInode->getName();
	path += currentInode->getType();

	currentDirectory = path;
}

bool FileSystem::changeDir(char * folderPath)
{
	Inode *tempNode = nullptr;
	tempNode = walkDir(folderPath);
	if (tempNode != nullptr) changeCurrentInode(tempNode);
	else return false;
	return true;
}
bool FileSystem::removeFolder(char * path)
{
	bool removed = false;
	Inode *removalNode = nullptr;
	removalNode = walkDir(path);
	if (removalNode != nullptr)
	{
		Block parentBlock = mMemblockDevice.readBlock(removalNode->getParentHDDLoc());
		Inode * parentNode = new Inode(parentBlock);

		removed = parentNode->removeNodeAt(this->getIndexOfNodeWithName(removalNode->getName(), parentNode));
		
		mMemblockDevice.writeBlock(parentNode->getHDDLoc(), parentNode->toCharArray());

		refreshCurrentInode();

		delete removalNode;
		delete parentNode;



	}

	return removed;
}
/* Compares all the names in the current Inode
Return false if name found */
bool FileSystem::isNameUnique(const char * name, const Inode* inode) const
{
	//Read available blocks and store names
	int numberOfBlocks = inode->getNrOfBlocks();
	std::string* names = new std::string[numberOfBlocks];
	for(int i = 2; i < numberOfBlocks; i++)
		if (inode->isBlockUsed(i))
		{
			Block currentBlock = mMemblockDevice.readBlock(inode->getBlockIndex(i));
			Inode curInode(currentBlock);
			names[i] = curInode.getName();
			if (name == names[i])
			{
				delete[] names;
				return false;
			}

		}
	delete[] names;
	return true;
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

int FileSystem::getIndexOfNodeWithName(const char * name, const Inode * inode) const
{
	//Read available blocks and store names
	int returnIndex = -1;
	int numberOfBlocks = inode->getNrOfBlocks();
	std::string* names = new std::string[numberOfBlocks];
	for (int i = 2; i < numberOfBlocks && returnIndex == -1; i++)
		if (inode->isBlockUsed(i))
		{
			Block currentBlock = mMemblockDevice.readBlock(inode->getBlockIndex(i));
			Inode curInode(currentBlock);
			names[i] = curInode.getName();
			if (name == names[i])
			{
				delete[] names;
				names = nullptr;
				returnIndex = i;
			}

		}
	if (names != nullptr) delete[] names;
	return returnIndex;
}

void FileSystem::refreshCurrentInode()
{
	// Update currentInode 
	Block currentBlock = mMemblockDevice.readBlock(currentInode->getHDDLoc());

	delete currentInode;
	currentInode = new Inode(currentBlock);
}

char * FileSystem::stringToCharP(const std::string& string) const
{
	char* newChar = new char[string.length()+1];
	//std::copy(string.begin(), string.end(), newChar);
	for (int i = 0; i < string.length(); i++)
		newChar[i] = string[i];
	newChar[string.length()] = '\0';
	return newChar;
}

void FileSystem::changeCurrentInode(Inode * newCur)
{
	//TODO: Less deallocating and more set functions. 
	delete currentInode;
	currentInode = new Inode(*newCur);
	delete newCur;

	// Updates name
	currentDirectory = currentInode->getName();
	currentDirectory += currentInode->getType();
}


/* Please insert your code */
