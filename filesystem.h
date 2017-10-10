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
	bool createFile(char* fileName, const char* content, int sizeInBytes);

    /* Creates a folder in the filesystem */
	bool createFolder(char* folderName);

    /* Removes a file in the filesystem */
    // removeFile(...);

	/*Copy target to destination*/
	bool copyTarget(char * target, char * destination);

    /* Removes a folder in the filesystem */
	bool removeFolder(char * path);

	void createImage(char* folderPath);
	bool readImage(char* folderPath);
    /* Function will move the current location to a specified location in the filesystem */
    // goToFolder(...);

    /* This function will get all the files and folders in the specified folder */
	bool listCopy(char* filepath, std::string& holder);
	/* This function will return the path to current working directory */
	std::string pwd();

    /* Add your own member-functions if needed */
	/* Return current INode name+type*/
	std::string currentDir() const;

	void setCurrentDirStr(const std::string& str, bool remove);

	bool changeDir(char* folderPath);
	
	//Global removal
	

private:
	// Help functions

	/* Compares all the names in the current Inode
	Return false if name found */
	bool isNameUnique(const char* name, const Inode* inode) const;


	/* Convert string to char* */
	char* stringToCharP(const std::string& string) const;

	/* Change current Inode */
	void changeCurrentInode(Inode* newCur);

	Inode* walkDir(char* folderPath);

	std::string dirNameJumper(int index);

	Inode* pathSolver(char* folderName, std::string*& folderNames, int& arraySize);

	/* Splits a char* into strings based on '/' */
	std::string* seperateSlashes(char* filepath, int& size) const;

	int getIndexOfNodeWithName(const char * name, const Inode* inode) const;

	void refreshCurrentInode();

	char* constChartoChar(const char* string) const;

	void init();

	bool copyRecursive(char * target, char * destination);
};
#endif // FILESYSTEM_H
