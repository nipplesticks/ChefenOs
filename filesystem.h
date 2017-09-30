#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "memblockdevice.h"
#include "Inode.h"


class FileSystem
{
private:
    MemBlockDevice mMemblockDevice;
	Inode* currentInode;
	std::string currentDirectory;

public:
    FileSystem();
    ~FileSystem();

    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */

    /* This function creates a file in the filesystem */
    // createFile(...)

    /* Creates a folder in the filesystem */
    void createFolder(char* folderName);

    /* Removes a file in the filesystem */
    // removeFile(...);

    /* Removes a folder in the filesystem */
    bool removeFolder();

    /* Function will move the current location to a specified location in the filesystem */
    // goToFolder(...);

    /* This function will get all the files and folders in the specified folder */
	std::string listDir() const;

    /* Add your own member-functions if needed */
	/* Return current INode name+type*/
	std::string currentDir() const;

	void setCurrentDirStr(const std::string& str, bool remove);

	bool changeDir2(char* folderPath);
	bool changeDir(char * folderPath);

private:
	// Help functions

	/* Compares all the names in the current Inode
	Return false if name found */
	bool isNameUnique(const char* name);

	Inode* walkDir(Inode* currentDirectory, char * next);

	/* Splits a char* into strings based on '/' */
	std::string* seperateSlashes(char* filepath, int& size) const;

	/* Convert string to char* */
	char* stringToCharP(const std::string& string) const;

	/* Change current Inode */
	void changeCurrentInode(Inode* newCur);

};

#endif // FILESYSTEM_H
