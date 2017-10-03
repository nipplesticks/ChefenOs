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
	bool createFile(char* fileName, char* content, int sizeInBytes);

    /* Creates a folder in the filesystem */
	bool createFolder(char* folderName);

    /* Removes a file in the filesystem */
    // removeFile(...);

    /* Removes a folder in the filesystem */
    bool removeFolder();

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

	/* Splits a char* into strings based on '/' */
	std::string* seperateSlashes(char* filepath, int& size) const;

	void init();
};
#endif // FILESYSTEM_H
