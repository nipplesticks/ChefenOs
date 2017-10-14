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

bool FileSystem::createFile(char * fileName, const char* content, int sizeInBytes)
{
	bool isCreated = false;

	int arraySize = 0;
	std::string* folders;
	Inode* tempNode = pathSolver(fileName, folders, arraySize);
	// Now i know if its relative or absolute path
	std::string filePathBeforeFile;
	for (int i = 0; i < arraySize - 1; i++)
	{
		filePathBeforeFile += "/" + folders[i];
	}
	Inode* currentHolder = nullptr;
	if (arraySize - 1 == 0)
	{
		currentHolder = tempNode;
	}
	else
	{
		char* fpbf_p = stringToCharP(filePathBeforeFile);
		createFolder(fpbf_p);

		currentHolder = walkDir(fpbf_p);
		delete[] fpbf_p;
		delete tempNode;

	}

	char* nodeType = constChartoChar("file");
	char* name = stringToCharP(folders[arraySize - 1]);

	// Check to see if there is any room left for the parent node AND if the name is unique
	if (currentHolder->getNrOfFreeBlocks() && isNameUnique(name, currentHolder))
	{
		// Create the fileNode
		int hddWriteIndex = currentHolder->getHDDadress(currentHolder->freeBlockInInode());

		// Now lets create the filenode
		Inode* fileNode = new Inode(nodeType, name, hddWriteIndex, currentHolder->getHDDLoc());

		// Gives the filenode access to ten blocks
		for (int i = 0; i < fileNode->getNrOfBlocks() - 2; i++)
			fileNode->setBlock(mMemblockDevice.getFreeHDDIndex());

		// Now is it time to check how many blocks the content require.
		int numberOfBlocks = (sizeInBytes / 512) + 1; 

		if (numberOfBlocks < fileNode->getNrOfFreeBlocks())
		{
			// Now is it time to fill the buffers with filecontent
			char** buffers = new char*[numberOfBlocks];
			for (int i = 0; i < numberOfBlocks; i++)
			{
				buffers[i] = new char[512];
			}

			int currentBlock = 0;
			do
			{
				for (int k = 0; k < 512; k++)
				{
					buffers[currentBlock][k] = content[k + (currentBlock * 512)];
				}
				currentBlock++;
			} while (currentBlock < numberOfBlocks - 1);

			int rest = sizeInBytes % 512;
			if (sizeInBytes != rest)
			{
				for (int k = 0; k < rest; k++)
				{
					buffers[currentBlock][k] = content[k+(currentBlock *512)];
				}
				buffers[currentBlock][rest] = '\0';
			}

			// Now is it time to write to the fileNode
			for (int i = 0; i < numberOfBlocks; i++)
			{
				fileNode->lockBlockAt(i + 2);
				mMemblockDevice.writeBlock(fileNode->getHDDadress(i + 2), buffers[i]);
			}

			fileNode->setDataSize(sizeInBytes);

			// Now is it time to write the fileNode to disk and parentNode
			currentHolder->lockFirstAvailableBlock();

			char* fileNodeContent = fileNode->toCharArray();
			char* currentNodeContent = currentHolder->toCharArray();

			mMemblockDevice.writeBlock(hddWriteIndex, fileNodeContent);
			mMemblockDevice.writeBlock(currentHolder->getHDDLoc(), currentNodeContent);

			delete[] fileNodeContent;
			delete[] currentNodeContent;

			refreshCurrentInode();

			isCreated = true;
			for (int i = 0; i < numberOfBlocks; i++)
			{
				delete[] buffers[i];
			}
			delete buffers;
		}
		
		delete fileNode;

	}
	else
	{
		delete[] nodeType;
		delete[] name;
	}
	delete[] folders;
	delete currentHolder;
	return isCreated;
	
}
/* IS NOT MEMORY EFFICIENT!!*/
bool FileSystem::append(char * sFP, char * dFP)
{
	bool result = false;
	Inode* sourceFile = walkDir(sFP);
	Inode* destFile = walkDir(dFP);

	if (sourceFile && destFile)
	{
		
		int nrOfBlocksNeeded	= sourceFile->getNrOfInUseBlocks() - 2,
			bufferSize			= nrOfBlocksNeeded * 512,
			bufferIndex			= 0;
		
		char* buffer1 = new char[bufferSize];
		// Fill buffer1 with sourceFile's content
		int nrOfBlocks = sourceFile->getNrOfBlocks();
		for (int i = 2; i < nrOfBlocks && bufferIndex != bufferSize; i++)
		{
			if (sourceFile->isBlockUsed(i))
			{
				Block curBlock = mMemblockDevice.readBlock(sourceFile->getHDDadress(i));
				for (int k = 0; k < 512 && curBlock[k] != '\0'; k++)
				{
					buffer1[bufferIndex] = '\0';
					buffer1[bufferIndex++] = curBlock[k];
				}
				
			}
		}
		buffer1[bufferIndex] = '\0';
		if (destFile->getNrOfFreeBlocks() > nrOfBlocksNeeded)
		{
			result = true;
			char** buffer2 = new char*[nrOfBlocksNeeded];
			for (int i = 0; i < nrOfBlocksNeeded; i++)
			{
				buffer2[i] = new char[512];
				int k = 0;
				for (; k < 512 && buffer1[k+(i*512)] != '\0'; k++)
				{
					buffer2[i][k] = '\0';
					buffer2[i][k] = buffer1[k + (i * 512)];
				}
				if(k == 512) 
					buffer2[i][k-1] = '\0';
				else
					buffer2[i][k] = '\0';
			}
			int blockCounter = 0;
			for (int i = 2; i < nrOfBlocks && blockCounter != nrOfBlocksNeeded; i++)
			{
				if (!destFile->isBlockUsed(i))
				{
					destFile->lockBlockAt(i);
					mMemblockDevice.writeBlock(destFile->getHDDadress(i), buffer2[blockCounter++]);
				}
			}
			destFile->setDataSize(destFile->getDataSize() + bufferIndex);
			char* fileNodeContent = destFile->toCharArray();

			mMemblockDevice.writeBlock(destFile->getHDDLoc(), fileNodeContent);
			
			delete[] fileNodeContent;
			
			for (int i = 0; i < nrOfBlocksNeeded; i++)
				delete[] buffer2[i];
			
			delete[] buffer2;
			
			
		}

		delete[] buffer1;
	}
	if (sourceFile) delete sourceFile;
	if (destFile) delete destFile;
	return result;
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
	// 
	currentHolder = pathSolver(folderName, folderNames, arraySize);

	while (arrayIndex != arraySize)
	{	
		// Does there exist a folder with this name in the current directory?
		if (isNameUnique(folderNames[arrayIndex].c_str(), currentHolder) && folderNames[arrayIndex] != "..")
		{
			int hddWriteIndex = currentHolder->getHDDadress(currentHolder->freeBlockInInode());
			if (hddWriteIndex != -1)
			{

				char* currentFolder = stringToCharP(folderNames[arrayIndex]);
				char* currentType = stringToCharP(std::string("/"));

				Inode *newInode = new Inode(currentType, currentFolder, hddWriteIndex, currentHolder->getHDDLoc());
				//int* freeBlocks = mMemblockDevice.getFreeBlockAdresses2();
				for (int i = 0; i < newInode->getNrOfBlocks() - 2; i++)
					newInode->setBlock(mMemblockDevice.getFreeHDDIndex());
				//delete[] freeBlocks;

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
	bool foundFile = false;
	if (file.good())
	{
		foundFile = true;

		// Calculate how many characters in file
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();

		// Reset filepointer to begining
		file.seekg(0, std::ios_base::beg);
		
		char* buffer = new char[size];

		// Fill buffer with file content
		file.read(buffer, size);
		file.close();
		
		// Position in file
		int bufferIndex = 0;
		std::string container;
		// Read nrOfBlocks
		container = readFileLine(buffer, bufferIndex);
		int nrOfBlocks = std::stoi(container);
		char* blockContent = new char[512];
		
		mMemblockDevice.reset();
		for (int i = 0; i < nrOfBlocks; i++)
		{
			for (int blockIndex = 0; blockIndex < 512;)
				blockContent[blockIndex++] = buffer[bufferIndex++];
			
			mMemblockDevice.writeBlock(i, blockContent);
		}
		// now is it time to gather all the free blocks
		int* freeBlocks = new int[nrOfBlocks];
		int arraySize = 0;
		while((1 + size)> bufferIndex)
		{
			container = readString(buffer, bufferIndex, size);
			if (strcmp(container.c_str(), ""))
			{
				freeBlocks[arraySize++] = std::stoi(container);
			}
			
		}

		mMemblockDevice.createList(freeBlocks, arraySize);
		
		delete[] freeBlocks;
		delete[] buffer;
		delete[] blockContent;
		refreshCurrentInode();
		
	}
	return foundFile;
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
	holder += "Listing directory\nName\t\tType\t\tPermissions\t\tSize\n";
	for (int i = 2; i < nrOfBlocks; i++)
	{
		blockIndexes[i] = tempNode->getHDDadress(i);
		if (tempNode->isBlockUsed(i))
		{
			Inode printer(mMemblockDevice.readBlock(blockIndexes[i]));
			// Name
			holder += printer.getName();
			holder += "\t\t";
			
			// Type
			const char* type = printer.getType();
			if (type[0] == '/')
				holder += "DIR\t\t";
			else
				holder += "FILE\t\t";
			// Permisions
			holder += "\t\t";

			// Size 
			holder += std::to_string(printer.getDataSize()) + " byte"; 
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

/* Fully working changeDir method with support for multiple slashes 
Remember to delete return pointer*/
Inode* FileSystem::walkDir(char * folderPath) const
{
	// Initialize variables
	int nrOfInodeBlocks = currentInode->getNrOfBlocks();
	int stringSize = 0, stringIndex = 0;
	std::string* folder = nullptr;
	Inode* tempNode = nullptr;

	tempNode = pathSolver(folderPath, folder, stringSize);

	//Locate folder in current Inode table
	for (int i = 2; i < nrOfInodeBlocks && (stringSize != stringIndex); i++)
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
		else if (folder[stringIndex] == ".")
		{
			delete tempNode;
			tempNode = new Inode(*currentInode);
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
	else return dirNameJumper(name.getParentHDDLoc()) + name.getName() + name.getType();
}

Inode * FileSystem::pathSolver(char * folderName, std::string*& folderNames, int& arraySize) const
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

std::string FileSystem::toTreeFormat() const
{

	std::string returnString = "/\n";
	int width = 0;
	int undone = 0;
	int* counter = new int[2];
	counter[0] = 0;
	counter[1] = 0;

	traverseDirectory(currentInode, width,undone,false, returnString,counter);

	returnString += "\n";
	// Grammer
	if (counter[0] > 1)
		returnString += std::to_string(counter[0]) + " directories, ";
	else
		returnString += std::to_string(counter[0]) + " directory, ";

	if (counter[1] > 1)
		returnString += std::to_string(counter[1]) + " files";
	else
		returnString += std::to_string(counter[1]) + " file";

	delete[] counter;
	return returnString;
}

/* returnValue meanings:
0: file not found
1: fileFound
-1: its a directory
*/
int FileSystem::getFileContent(char * target, std::string& content) const
{

	int returnValue = 0;

	Inode* currentHolder = walkDir(target);
	
	if (currentHolder)
	{
		if (currentHolder->getType()[0] != '/')
		{

			int nrOfBlocks = currentHolder->getNrOfBlocks();
			for (int i = 2; i < nrOfBlocks; i++)
			{
				if (currentHolder->isBlockUsed(i))
				{
					Block partOfFile = mMemblockDevice.readBlock(currentHolder->getHDDadress(i));
					for (int contentIndex = 0; (contentIndex < 512) && partOfFile.toString()[contentIndex] != '\0'; contentIndex++)
					{
						std::string blockContent = partOfFile.toString();
						content += blockContent[contentIndex];
					}
					returnValue = 1;
				}
			}
			content += '\n';
		}
		else
		{
			returnValue = -1;
		}
		
	}
	delete currentHolder;
	return returnValue;
}

bool FileSystem::copyRecursive(Inode * targetNode, Inode * destinationNode)
{
	bool returnValue = false;
	if (targetNode != nullptr &&
		destinationNode != nullptr &&
		isNameUnique(targetNode->getName(), destinationNode))
	{
		targetNode->setParentHDDLoc(destinationNode->getHDDLoc());
		//Ta fram 10 adresser till disken
		int * targetSubAdresses = mMemblockDevice.get10FreeBlockAdresses();
		//Loopa number of adresses i target
		int nrOfBlocks = targetNode->getNrOfBlocks();
		if (targetNode->getType()[0] == '/')
		{
			for (int i = 2; i < nrOfBlocks; i++)
			{
				int oldAdress = targetNode->changeBlockAdress(i, targetSubAdresses[i - 2]);
				//Kolla alla adresser som är skrivna
				if (targetNode->isBlockUsed(i))
				{
					Inode * subTarget = new Inode(mMemblockDevice.readBlock(oldAdress));
					subTarget->setHDDLoc(targetNode->getHDDadress(i));
					returnValue = copyRecursive(subTarget, new Inode(*targetNode));
				}
			}
		}
		else
		{
			Block blocks[10];
			int counter = 0;
			for (int i = 2; i < nrOfBlocks; i++)
			{
				int oldAdress = targetNode->changeBlockAdress(i, targetSubAdresses[i - 2]);
				//Kolla alla adresser som är skrivna
				if (targetNode->isBlockUsed(i))
				{
					blocks[counter++] = mMemblockDevice.readBlock(oldAdress);
				}
			}
			counter = 0;
			//Write to disk
			for (int i = 2; i < nrOfBlocks; i++)
			{
				if (targetNode->isBlockUsed(i))
				{
					mMemblockDevice.writeBlock(targetNode->getHDDadress(i), blocks[counter++].toString());
				}
			}

		}

		delete[] targetSubAdresses;

		//Skriv till disk
		char * targetChar = targetNode->toCharArray();
		char * destinationChar = destinationNode->toCharArray();
		mMemblockDevice.writeBlock(targetNode->getHDDLoc(), targetChar);
		mMemblockDevice.writeBlock(destinationNode->getHDDLoc(), destinationChar);

		delete[] targetChar;
		delete[] destinationChar;

		returnValue = true;
	}

	if (targetNode != nullptr)
	{
		delete targetNode;
		targetNode = nullptr;
	}

	if (destinationNode != nullptr)
	{
		delete destinationNode;
		destinationNode = nullptr;
	}

	return returnValue;
}

bool FileSystem::copyTarget(char * target, char * destination)
{
	bool result = false;
	//Få tag i target noden och gör en kopia på den
	Inode * targetNode = walkDir(target);
	//Gå till destinationen
	Inode * destinationNode = walkDir(destination);
		
	if (targetNode != nullptr && destinationNode != nullptr)
	{
		if (isNameUnique(targetNode->getName(), destinationNode));
		{
			char * name = constChartoChar(targetNode->getName());
			std::string nameAsString = name;
			int indexOfDot = nameAsString.find_last_of('.');
			if (indexOfDot != -1)
			{
				std::string fileType = "";
				int lengthOfString = nameAsString.length();
				for (int i = indexOfDot; i < lengthOfString; i++)
					fileType += nameAsString[i];
				for (int i = indexOfDot; i < lengthOfString; i++)
					nameAsString.pop_back();
				nameAsString += "_copy" + fileType;
			}
			else
			{
				nameAsString += "_copy";
			}
			char * newName = stringToCharP(nameAsString);
			targetNode->setName(newName);

			//delete[] newName;
			delete[] name;
		}
		int indexInDestinationArray = destinationNode->freeBlockInInode();
		int targetNewAdressOnHDD = destinationNode->getHDDadress(indexInDestinationArray);
		targetNode->setHDDLoc(targetNewAdressOnHDD);

		//Sätt target på destinationen
		destinationNode->lockFirstAvailableBlock();

		result = copyRecursive(targetNode, destinationNode);

		refreshCurrentInode();
	}

	return result;
}

bool FileSystem::removeFile(char* fileName, Inode* parentNode)
{
	bool removed = false;
	if (parentNode->getNrOfFreeBlocks())
	{
		int numberOfBlocks = parentNode->getNrOfBlocks();
		for (int i = 2; i < numberOfBlocks; i++)
		{
			if (parentNode->isBlockUsed(i))
			{
				int hddAddressToRemove = parentNode->getHDDadress(i);
				Block nodeBlock = mMemblockDevice.readBlock(hddAddressToRemove);
				Inode temp(nodeBlock);
				
				if (!strcmp(temp.getName(), (const char*)fileName)) 
				{
					int numberOfAdresses = temp.getNrOfBlocks();
					int* hddAdresses = new int[numberOfAdresses];
					int counter = 0;
					for (int curAdr = 2; curAdr < numberOfAdresses; curAdr++)
					{
						int address = temp.getHDDadress(curAdr);
						if (address != -1)
							hddAdresses[counter++] = address;
					}
					// After obtaining all the addresses we will now give them back to mMemblockDevice
					for (int j = counter - 1; j >= 0; j--)
						mMemblockDevice.insertBlockIndex(hddAdresses[j]);
					
					// Now we cut the connection with the parent node
					parentNode->unlockBlockAt(i);
					char* parentNodeContent = parentNode->toCharArray();

					mMemblockDevice.writeBlock(parentNode->getHDDLoc(), parentNodeContent);

					delete[] parentNodeContent;
					delete[] hddAdresses;
					refreshCurrentInode();
					removed = true;
				}
			}
		}
	}
	return removed;
}

bool FileSystem::remove(char * path)
{
	bool result = false;
	Inode* nodeToDelete = walkDir(path);
	if (nodeToDelete)
	{
		const char* name = nodeToDelete->getName();
		Block parentBlock = mMemblockDevice.readBlock(nodeToDelete->getParentHDDLoc());
		Inode* parentNode = new Inode(parentBlock);

		if (!strcmp(nodeToDelete->getType(), "/"))
		{
			clearFolder(nodeToDelete);

			int nrOfAdresses = nodeToDelete->getNrOfBlocks();
			int counter = 0;
			int* adresses = new int[nrOfAdresses];
			for (int curAdr = 2; curAdr < nrOfAdresses; curAdr++)
			{
				adresses[counter++] = nodeToDelete->getHDDadress(curAdr);
			}
			// After obtaining all the addresses we will now give them back to mMemblockDevice
			for (int j = counter - 1; j >= 0; j--)
				mMemblockDevice.insertBlockIndex(adresses[j]);

			for (int i = 2; i < nrOfAdresses && !result; i++)
			{
				if (parentNode->isBlockUsed(i))
				{
					Block childBlock = mMemblockDevice.readBlock(i);
					Inode temp(childBlock);

					if (!strcmp(name, nodeToDelete->getName()))
					{
						parentNode->unlockBlockAt(i);
						result = true;
					}
				}
			}
			
			char* parentNodeContent = parentNode->toCharArray();

			mMemblockDevice.writeBlock(parentNode->getHDDLoc(), parentNodeContent);

			delete[] adresses;
			delete[] parentNodeContent;
			
		}
		else
			result = removeFile(path, parentNode);

		delete parentNode;
	}

	refreshCurrentInode();
	delete nodeToDelete;
	return result;
}

bool FileSystem::clearFolder(Inode* tbrNode)
{
	bool removed = false;
	int nrOfBlocks = tbrNode->getNrOfBlocks();
	for (int i = 2; i < nrOfBlocks; i++)
	{
		if (tbrNode->isBlockUsed(i))
		{
			Block currentBlock = mMemblockDevice.readBlock(tbrNode->getHDDadress(i));
			Inode temp(currentBlock);
			if (!strcmp(temp.getType(), "file"))
			{
				char* name = constChartoChar(temp.getName());
				removeFile(name, tbrNode);
				delete[] name;
			}
			else
			{
				Inode* childFolder = new Inode(currentBlock);
				clearFolder(childFolder);
				int nrOfAdresses = childFolder->getNrOfBlocks();
				int counter = 0;
				int* adresses = new int[nrOfAdresses];
				for (int curAdr = 2; curAdr < nrOfAdresses; curAdr++)
				{
					adresses[counter++] = childFolder->getHDDadress(curAdr);
				}
				// After obtaining all the addresses we will now give them back to mMemblockDevice
				for (int j = counter - 1; j >= 0; j--)
					mMemblockDevice.insertBlockIndex(adresses[j]);

				tbrNode->unlockBlockAt(i);
				char* parentNodeContent = tbrNode->toCharArray();

				mMemblockDevice.writeBlock(tbrNode->getHDDLoc(), parentNodeContent);
				
				delete[] adresses;
				delete[] parentNodeContent;
				removed = true;

				delete childFolder;
			}
		}
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

std::string FileSystem::readString(char * buffer, int & bufferIndex, int size)
{

	std::string line = "";
	while (buffer[bufferIndex] != ' ' && size >= bufferIndex)
	{
		line += buffer[bufferIndex++];
	}
	bufferIndex++; // Jump over ' ' 
	return line;
}

void FileSystem::traverseDirectory(Inode * current, int & width, int& undone,bool last, std::string & content, int*& counter) const
{
	// Calculate how many entries in the map
	int nrOfUsed = 0, nrOfBlocks = current->getNrOfBlocks();
	for (int i = 2; i < nrOfBlocks; i++)
		if (current->isBlockUsed(i)) nrOfUsed++;

	for (int i = 2; i < nrOfBlocks && nrOfUsed != 0; i++)
	{
		if (current->isBlockUsed(i))
		{
			// To read the name
			Block curBlock = mMemblockDevice.readBlock(current->getHDDadress(i));
			Inode* tempNode = new Inode(curBlock);
			if (nrOfUsed != 1) // This isnt the last entry
			{
				/*if (width)	content += "|   ";*/
				for (int k = 0; k < undone; k++)
				{

						content += "|   ";
				}
				for(int i = 0; i < (width- undone);i++)
					content += "    ";
				content += "L__ ";
				content += tempNode->getName();	
				if(!strcmp(tempNode->getType(), "/") && !last) 
					undone++;
			}
			else // Its the last entry
			{
				for (int i = 0; i < undone; i++)
				{
					content += "|   ";
				}
				for(int i = 0; i < (width - undone); i++)
					content += "    ";
				
				/*if(width) content += "|   ";
				for (int k = 0; k < width; k++)
				{
					if(width > undone)
						content += "    ";	
					else
						content += "|   ";
				}*/
				content += "L__ ";
				content += tempNode->getName();
				
				last = true;
			}

			if (!strcmp(tempNode->getType(), "/")) // If its a directory
			{
				counter[0]++;
				content += "/\n";
				width++; // Increase width of the tree as we enter a folder
				traverseDirectory(tempNode, width, undone,last, content, counter);
				if (width == undone) undone--;
				width--; // Decrease widht of the tree as we leave a folder
			}
			else
			{
				counter[1]++;
				content += "\n";
			}
			nrOfUsed--;
			delete tempNode;

		}
	}
	//undone--;

}

char * FileSystem::stringToCharP(const std::string& string) const
{
	char* newChar = new char[string.length()+1];
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