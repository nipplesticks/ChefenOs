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
	
	/* Returns the entire virtual harddrive in a string*/
	std::string printHDD();

    /* Formats the harddrive by filling every block with Zero's
	And creates a empty root directory*/
	void formatHDD();

    /* Creates a file in the filesystem 
	Returns:
		1 = Success
		0 = Failed
	   -1 = Folder write denied
	   -2 = Folder read denied
	*/
	int createFile(char * fileName, const char* content, int sizeInBytes);
	/* Retrieves data from file "sfp" and writes it to the end of file "dpf"
	Returns:
		1 = Success
		0 = Failed
	   -1 = source file read denied
	   -2 = destination file read denied
	   -3 = destination file write denied
	   */
	int append(char* sFP, char* dFP);

	/* Moves a folder or file to a given destination. If the destination is not
	available the given source experience a name change instead */
	int move(char* sFP, char* dFP);

    /* Creates a folder in the filesystem 
	Returns:
		1 = Success
		0 = Failed
	   -1 = Parentfolder read denied
	   -2 = Parentfolder write denied*/
	int createFolder(char* folderName);

	/* Copy target folder/file to destination folder
	Returns:
		1 = Success
		0 = Failed
	   -1 = "target" read denied
	   -2 = "target" write denied
	   -3 = "destination" folder read denied
	   -4 = "destination" folder write denied
	   -5 = "destination" is a file*/
	int copyTarget(char * target, char * destination);

	/* Removes the folder/file given its relative or absolute path
	Returns:
		1 = Success
		0 = Failed
	   -1 = Folder/file write denied
	   -2 = Parent folder write denied
	*/
	int remove(char* path);

	/* Creates a file on the real harddrive containing the image of the
	current filesystem*/
	void createImage(char* path);

	/* Reads a image of an earlier created filesystem and replaces the current
	one. 
	Returns:
		true = Success
		false = Failed to open the file*/
	bool readImage(char* path);

    /* This function will get all the files and folders in the specified folder */
	bool listCopy(char* filepath, std::string& holder);
	/* This function will return the path to current working directory */
	std::string pwd();

    /* Add your own member-functions if needed */
	/* Return current INode name+type*/
	std::string currentDir() const;

	void setCurrentDirStr(const std::string& str, bool remove);

	int changeDir(char* folderPath);

	std::string toTreeFormat() const;
	
	int chmod(char* rights, char* path);
	/* returnValue meanings:
	0: file not found
	1: fileFound
	-1: its a directory
	*/
	int getFileContent(char * target, std::string& content) const;

private:
	// Help functions

	/* Compares all the names in the current Inode
	Return false if name found */
	bool isNameUnique(const char* name, const Inode* inode) const;

	/* Convert string to char* */
	char* stringToCharP(const std::string& string) const;

	bool clearFolder(Inode* tbrNode);
	bool removeFile(char* fileName, Inode* parentNode);

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
