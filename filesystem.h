#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "memblockdevice.h"
#include "Inode.h"
#include <fstream>
#include <string.h>

class FileSystem
{
private:
    MemBlockDevice mMemblockDevice;
	Inode* currentInode;
	std::string currentDirectory;

public:
    FileSystem();
    ~FileSystem();
	//DEBUG METHOD
	std::string lol();
    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */
	void formatHDD();
    /* This function creates a file in the filesystem */
	bool createFile(char * fileName, const char* content, int sizeInBytes);

    /* Creates a folder in the filesystem */
	bool createFolder(char* folderName);

    /* Removes a file in the filesystem */
    // removeFile(...);

	/*Copy target to destination*/
	bool copyTarget(char * target, char * destination);

    /* Removes a folder in the filesystem */
	bool removeFolder(char * path);

	bool removeFile(char* fileName, Inode* parentNode);

	void createImage(char* folderPath);

	bool readImage(char* folderPath);

    /* This function will get all the files and folders in the specified folder */
	bool listCopy(char* filepath, std::string& holder);
	/* This function will return the path to current working directory */
	std::string pwd();

    /* Add your own member-functions if needed */
	/* Return current INode name+type*/
	std::string currentDir() const;

	void setCurrentDirStr(const std::string& str, bool remove);

	bool changeDir(char* folderPath);

	std::string toTreeFormat() const;

	//Global removal
	
	std::string getFileContent(char * target) const;

private:
	// Help functions

	/* Compares all the names in the current Inode
	Return false if name found */
	bool isNameUnique(const char* name, const Inode* inode) const;

	/* Convert string to char* */
	char* stringToCharP(const std::string& string) const;

	/* Change current Inode */
	void changeCurrentInode(Inode* newCur);
	
	/* Returns the Inode associated with given folderpath */
	Inode* walkDir(char* folderPath) const;

	std::string dirNameJumper(int index);

	/* Determines if the path is absolute or relative and stores each dir in string* */
	Inode* pathSolver(char* folderName, std::string*& folderNames, int& arraySize) const;

	/* Splits a char* into strings based on '/' */
	std::string* seperateSlashes(char* filepath, int& size) const;

	int getIndexOfNodeWithName(const char * name, const Inode* inode) const;

	void refreshCurrentInode();
	
	/* Converts a const char into a char */
	char* constChartoChar(const char* string) const;

	std::string readFileLine(char* buffer, int& bufferIndex);
	std::string readString(char* buffer, int& bufferIndex, int size);

	void traverseDirectory(Inode* current, int& width, int& undone,bool last, std::string& content,int*& counter) const;

	void init();

	bool copyRecursive(Inode * target, Inode * destination);

	
};
#endif // FILESYSTEM_H
