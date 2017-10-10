#include "filesystem.h"
#define _CRTDBG_MAP_ALLOC
#include<iostream>
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
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
bool FileSystem::createFile(char * fileName, const char* content, int sizeInBytes)
{

	
	int hddWriteIndex = currentInode->getHDDadress(currentInode->freeBlockInInode());

	char* nodeType = constChartoChar("file");
 	Inode* fileNode = new Inode(nodeType, fileName, hddWriteIndex, currentInode->getHDDLoc());

	int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();

	for (int i = 2; i < fileNode->getNrOfBlocks(); i++)
		fileNode->setBlock(freeBlocks[i]);
	delete[] freeBlocks;


	int freeNodewriteIndex = fileNode->getHDDadress(fileNode->freeBlockInInode());

	currentInode->lockFirstAvailableBlock();
	fileNode->lockFirstAvailableBlock();

	char* fileNodeContent = fileNode->toCharArray();
	char* currentNodeContent = currentInode->toCharArray();
	mMemblockDevice.writeBlock(freeNodewriteIndex, content);
	mMemblockDevice.writeBlock(hddWriteIndex, fileNodeContent);
	mMemblockDevice.writeBlock(currentInode->getHDDLoc(), currentNodeContent);
	
	delete[] fileNodeContent;
	delete[] currentNodeContent;
	delete fileNode;
	refreshCurrentInode();

	return false;
}

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

	currentHolder = pathSolver(folderName, folderNames, arraySize);

	while (arrayIndex != arraySize)
	{	
		// Does there exist a folder with this name in the current directory?
		if (isNameUnique(folderNames[arrayIndex].c_str(), currentHolder))
		{
			int hddWriteIndex = currentHolder->getHDDadress(currentHolder->freeBlockInInode());
			if (hddWriteIndex != -1)
			{

				char* currentFolder = stringToCharP(folderNames[arrayIndex]);
				char* currentType = stringToCharP(std::string("/"));
				Inode *newInode = new Inode(currentType, currentFolder, hddWriteIndex, currentHolder->getHDDLoc());
				int* freeBlocks = mMemblockDevice.getFreeBlockAdresses();
				for (int i = 2; i < newInode->getNrOfBlocks(); i++)
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

				delete currentHolder;
				currentHolder = newInode;
				arrayIndex++;
			}/* If it is not the destination folder yet, we will instead traversel into the folder and continue.
				This enables the option to create a folder when you are not inside it */
		}
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
		// Calculate how many characters in file
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();

		// Reset filepointer to begining
		file.seekg(0, std::ios_base::beg);
		
		char* buffer = new char[size+1];
		buffer[size] = '\0';

		// Fill buffer with file content
		file.read(buffer, size);
		file.close();
		
		// Position in file
		int bufferIndex = 0;
		std::string container;
		// Read free Pointer
		container = readFileLine(buffer, bufferIndex);
		int freePointer = std::stoi(container);
		// Read nrOfBlocks
		container = readFileLine(buffer, bufferIndex);
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
		blockIndexes[i] = tempNode->getHDDadress(i);
		if (tempNode->isBlockUsed(i))
		{
			Inode printer(mMemblockDevice.readBlock(blockIndexes[i]));
			holder += printer.getName();
			// Folders print / and files dont
			if(printer.getType()[0] =='/') holder += printer.getType();
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

	tempNode = pathSolver(folderPath, folder, stringSize);

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
			Block curBlock = mMemblockDevice.readBlock(tempNode->getHDDadress(i));
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
	if (index == 0) return "/";	//Root dir reached
	else
		return dirNameJumper(name.getParentHDDLoc()) + name.getName() + name.getType();
}

Inode * FileSystem::pathSolver(char * folderName, std::string*& folderNames, int& arraySize)
{
	Inode* returnNode = nullptr;
	// Relative or absolute path?
	if (folderName[0] == '/')
	{
		Block rootBlock = mMemblockDevice.readBlock(0);
		returnNode = new Inode(rootBlock);
		folderNames = seperateSlashes(++folderName, arraySize);
	}
	else
	{
		returnNode = new Inode(*currentInode);
		folderNames = seperateSlashes(folderName, arraySize);
	}
	return returnNode;
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

bool FileSystem::copyTarget(char * target, char * destination)
{
	bool copySucceed = false;
	
	//Få tag i target noden och gör en kopia på den
	Inode * targetNode = walkDir(target);

	//Gå till destinationen och se till att det är en mapp
	Inode * destinationNode = walkDir(destination);
	if (targetNode != nullptr &&
		destinationNode != nullptr &&
		*destinationNode->getType() == '/' && 
		isNameUnique(targetNode->getName(), destinationNode))
	{
		//Ändra parent och hdd location i target
		targetNode->setParentHDDLoc(destinationNode->getHDDLoc());
		int indexInDestinationArray = destinationNode->freeBlockInInode();
		int targetNewAdressOnHDD = destinationNode->getHDDadress(indexInDestinationArray);
		targetNode->setHDDLoc(targetNewAdressOnHDD);
		
		//Sätt target på destinationen
		destinationNode->lockFirstAvailableBlock();

		//Skriv till disk
		char * targetChar = targetNode->toCharArray();
		char * destinationChar = destinationNode->toCharArray();
		mMemblockDevice.writeBlock(targetNewAdressOnHDD, targetChar);
		mMemblockDevice.writeBlock(destinationNode->getHDDLoc(), destinationChar);
		
		delete[] targetChar;
		delete[] destinationChar;


		refreshCurrentInode();
		copySucceed = true;
	}

	if (targetNode != nullptr) delete targetNode;
	if (destinationNode != nullptr) delete destinationNode;
	


	return copySucceed;
}

bool FileSystem::removeFolder(char * path)
{
	bool removed = false;
	Inode *removalNode = nullptr;
	
	removalNode = walkDir(path); //gets Inode which points to folder on disk
	if (removalNode != nullptr) //if exists
	{
		Block parentBlock = mMemblockDevice.readBlock(removalNode->getParentHDDLoc()); //Gets parent inode to the removal inode
		Inode * parentNode = new Inode(parentBlock);

		//Gets index in blockedUsed array with name of the removal node. Sets this index to false.
		removed = parentNode->removeNodeAt(this->getIndexOfNodeWithName(removalNode->getName(), parentNode));

		char * parentNodeAsChar = parentNode->toCharArray();
		mMemblockDevice.writeBlock(parentNode->getHDDLoc(), parentNodeAsChar);
		delete[] parentNodeAsChar;

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
			Block currentBlock = mMemblockDevice.readBlock(inode->getHDDadress(i));
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
			Block currentBlock = mMemblockDevice.readBlock(inode->getHDDadress(i));
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

char * FileSystem::constChartoChar(const char * string) const
{
	int charSize = 0;
	while (string[charSize++] != '\0');
	char* returnChar = new char[charSize];
	for (int i = 0; i < charSize; i++)
		returnChar[i] = string[i];

	return returnChar;
}

std::string FileSystem::readFileLine(char * buffer, int & bufferIndex)
{
	std::string line;
	while (buffer[bufferIndex] != '\r')
	{
		line += buffer[bufferIndex++];
	}
	bufferIndex += 2; // Jump over /r/n
	return line;
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
