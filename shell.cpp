#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>
#include <sstream>
#include<string>
#include "filesystem.h"


const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 18;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help","clear", "tree",
	"chmod"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
char* getCommandsAsChar(const std::string &command, int expectedCommands, int nrOfCommands);
void clearScr();

void list(std::string  commandArr[8], int nrOfCommands, FileSystem &fs);

void createFile(std::string  commandArr[8], int nrOfCommands, FileSystem &fs);

void getContent(std::string  commandArr[8], int nrOfCommands, FileSystem &fs);

void createImage(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void restoreImage(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void remove(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void copy(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void append(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void move(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void makeDir(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void changeDir(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

void chmod(std::string commandArr[8], int nrOfCommands, FileSystem &fs);

std::string advancedEditor();

std::string help();

/* More functions ... */

int main(void) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	FileSystem fs;
	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "Chefen@Computer";    // Change this if you want another user to be displayed
    bool bRun = true;

    do {
        std::cout << user << ":" << fs.pwd() << "\n$ ";
		getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {

            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

			case 0: //quit E DONE
				bRun = quit();                
                break;
            case 1: // format E DONE
				fs.formatHDD();
                break;
            case 2: // ls E DONE
				list(commandArr, nrOfCommands, fs);
                break;
            case 3: // create E
				createFile(commandArr, nrOfCommands, fs);
                break;
            case 4: // cat E
				getContent(commandArr, nrOfCommands, fs);
                break;
            case 5: // createImage E DONE
				createImage(commandArr, nrOfCommands, fs);
                break;
            case 6: // restoreImage E DONE
				restoreImage(commandArr, nrOfCommands, fs);
                break;
            case 7: // rm E
				remove(commandArr, nrOfCommands, fs);
                break;
            case 8: // cp E DONE
				copy(commandArr, nrOfCommands, fs);
                break;
            case 9: // append
				append(commandArr, nrOfCommands, fs);
				break;
            case 10: // mv
				move(commandArr, nrOfCommands, fs);
				break;
            case 11: // mkdir E	DONE		
				makeDir(commandArr, nrOfCommands, fs);
                break;
            case 12: // cd E DONE
				changeDir(commandArr, nrOfCommands, fs);
                break; 
            case 13: // pwd E DONE
				std::cout << fs.pwd() << std::endl;
                break;
            case 14: // help DONE
                std::cout << help() << std::endl;
                break;
			case 15: // clear DONE
				clearScr();
				break;
			case 16: // tree
				std::cout << fs.toTreeFormat() << std::endl;
				break;
			case 17: // chmod
				chmod(commandArr, nrOfCommands, fs);
				break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
        }
    } while (bRun == true);

    return 0;
}
void chmod(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char * arg1 = getCommandsAsChar(commandArr[1], 3, nrOfCommands); // Accessrights
	char * arg2 = getCommandsAsChar(commandArr[2], 3, nrOfCommands); // Filepath
	if (arg1 && arg2)
	{
		int returnValue = fs.chmod(arg1, arg2);
		if (returnValue < 0)
		{
			switch (returnValue)
			{
			case -1:
				std::cout << "chmod: " << arg2 << ": No such file or directory\n";
				break;
			case -2:
				std::cout << "chmod: " << arg1 << ": Invalid permissions\n";
				break;
			}
		}
		delete[] arg1;
		delete[] arg2;
	}
	else
	{
		std::cout << "chmod <accessrights> <filepath>\n";
	}
}

void changeDir(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char *arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1 != nullptr)
	{
		int returnValue = fs.changeDir(arg1);
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "cd: " << arg1 << ": No such file or directory\n";
				break;
			case -1:
				std::cout << "cd: " << arg1 << ": Read permission denied\n";
				break;
			}
		}
		delete[] arg1;
	}
	else
	{
		std::cout << "cd <filepath>" << std::endl;
	}
}

void makeDir(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1 != nullptr)
	{
		int returnValue = fs.createFolder(arg1);
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "mkdir: cannot create directory '" << arg1 << "': File exists\n";
				break;
			case -1:
				std::cout << "mkdir: cannot create directory '" << arg1 << "': Parent folder read denied\n";
				break;
			case -2:
				std::cout << "mkdir: cannot create directory '" << arg1 << "': Parent folder write denied\n";
				break;
			}
		}
		delete[] arg1;
	}
	else
	{
		std::cout << "mkdir <folderpath>" << std::endl;
	}

}

void move(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 3, nrOfCommands);
	char* arg2 = getCommandsAsChar(commandArr[2], 3, nrOfCommands);
	if (arg1 && arg2)
	{
		int returnValue = fs.move(arg1, arg2);
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "mv: cannot access '" << arg1 << " or " << arg2 << "': No such file or directory\n";
				break;
			case -1:
				std::cout << "mv: " << arg1 << ": Permission to read denied\n";
				break;
			case -2:
				std::cout << "mv: " << arg1 << ": Permission to write denied\n";
				break;
			case -3:
				std::cout << "mv: " << arg2 << ": Permission to read denied\n";
				break;
			case -4:
				std::cout << "mv: " << arg2 << ": Permission to write denied\n";
				break;
			case -5:
				std::cout << "mv: " << arg2 << ": Cant move to file\n";
				break;

			}
		}
		delete[] arg1;
		delete[] arg2;
	}
	else
	{
		std::cout << "mv <target> <destination>\n";
	}
}

void append(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 3, nrOfCommands);
	char* arg2 = getCommandsAsChar(commandArr[2], 3, nrOfCommands);
	if (arg1 && arg2)
	{
		int returnValue = fs.append(arg1, arg2);
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "append: cannot access '" << arg1 << " or " << arg2 << "': No such file or directory\n";
				break;
			case -1:
				std::cout << "append: " << arg1 << ": Permission to read denied\n";
				break;
			case -2:
				std::cout << "append: " << arg2 << ": Permission to read denied\n";
				break;
			case -3:
				std::cout << "append: " << arg2 << ": Permission to write denied\n";
				break;

			}
		}
		delete[] arg1;
		delete[] arg2;
	}
	else
	{
		std::cout << "append <target> <destination>\n";
	}
}

void copy(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 3, nrOfCommands);
	char* arg2 = getCommandsAsChar(commandArr[2], 3, nrOfCommands);
	if (arg1 && arg2)
	{
		int returnValue = fs.copyTarget(arg1, arg2);
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "cp: cannot access '" << arg1 << " or " << arg2 << "': No such file or directory\n";
				break;
			case -1:
				std::cout << "cp: " << arg1 << ": Permission to read denied\n";
				break;
			case -2:
				std::cout << "cp: " << arg1 << ": Permission to write denied\n";
				break;
			case -3:
				std::cout << "cp: " << arg2 << ": Permission to read denied\n";
				break;
			case -4:
				std::cout << "cp: " << arg2 << ": Permission to write denied\n";
				break;
			case -5:
				std::cout << "cp: " << arg2 << ": Cant copy to file\n";

			}
		}
		delete[] arg1;
		delete[] arg2;
	}
	else
	{
		std::cout << "cp <target> <destination>\n";
	}
}

void remove(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1)
	{
		int returnValue = fs.remove(arg1);
		if (returnValue <  1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "rm: cannot access '" << arg1 << "': No such file or directory\n";
				break;
			case -1:
				std::cout << "rm: " << arg1 << ": Permission to read denied\n";
				break;
			case -2:
				std::cout << "rm: " << arg1 << ": Parent folder write denied\n";
				break;
			}
		}
		delete[] arg1;
	}
	else
	{
		std::cout << "rm <path>\n";
	}
}

void restoreImage(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1)
	{
		if (!fs.readImage(arg1))
		{
			std::cout << "restoreImage: cannot read '" << arg1 << "': No such file\n";
		}
		delete[] arg1;
	}
	else
	{
		std::cout << "restoreImage <folderpath>\n";
	}
}

void createImage(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1)
	{
		fs.createImage(arg1);
		delete[] arg1;
	}
	else
	{
		std::cout << "createImage <folderpath>\n";
	}
}

void getContent(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1)
	{
		std::string fileContent;
		int result = fs.getFileContent(arg1, fileContent);
		switch (result)
		{
		case -2:
			std::cout << "cat: " << arg1 << ": Permission to read denied\n";
			break;
		case -1:
			std::cout << "cat: " << arg1 << ": Is a directory\n";
			break;
		default:
		case 0:
			std::cout << "cat: " << arg1 << ": No such file or directory\n";
			break;
		case 1:
			std::cout << fileContent << std::endl;
			break;

		}
	}
	else
	{
		std::cout << "cat <filepath>" << std::endl;
	}
	delete[] arg1;
}

void createFile(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	if (arg1)
	{
		/*Wow! Such advanced! Much wow!
		─────────▄──────────────▄
		────────▌▒█───────────▄▀▒▌
		────────▌▒▒▀▄───────▄▀▒▒▒▐
		───────▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐
		─────▄▄▀▒▒▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐
		───▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀██▀▒▌
		──▐▒▒▒▄▄▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄▒▒▌
		──▌▒▒▐▄█▀▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐
		─▐▒▒▒▒▒▒▒▒▒▒▒▌██▀▒▒▒▒▒▒▒▒▀▄▌
		─▌▒▀▄██▄▒▒▒▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌
		─▌▀▐▄█▄█▌▄▒▀▒▒▒▒▒▒░░░░░░▒▒▒▐
		▐▒▀▐▀▐▀▒▒▄▄▒▄▒▒▒▒▒░░░░░░▒▒▒▒▌
		▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒░░░░░░▒▒▒▐
		─▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌
		─▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐
		──▀▄▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄▒▒▒▒▌
		────▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀
		───▐▀▒▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀
		──▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀▀*/
		std::string content = advancedEditor();
		int returnValue = fs.createFile(arg1, content.c_str(), content.length());
		if (returnValue < 1)
		{
			switch (returnValue)
			{
			case 0:
				std::cout << "create: cannot write file '" << arg1 << "': File already exist\n";
				break;
			case -1:
				std::cout << "create: cannot write file '" << arg1 << "': Write to folder denied\n";
				break;
			case -2:
				std::cout << "create: cannot write file '" << arg1 << "': Read folder denied\n";
				break;
			}

		}
	}
	else
	{
		std::cout << "createImage <folderpath>\n";
	}
	delete[] arg1;
}

void list(std::string commandArr[8], int nrOfCommands, FileSystem &fs)
{
	char* arg1 = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
	std::string content;
	if (fs.listCopy(arg1, content))
		std::cout << content << std::endl;
	else
		std::cout << "ls: cannot access '" << arg1 << "': No such file or directory\n";
	content = "";
	delete[] arg1;
}

void clearScr()
{
	if (system("clear")) system("cls"); // Funkar både på windows och linux
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
    std::stringstream ssin(userCommand);
    int counter = 0;
    while (ssin.good() && counter < MAXCOMMANDS) {
        ssin >> strArr[counter];
        counter++;
    }
    if (strArr[0] == "") {
        counter = 0;
    }
    return counter;
}

int findCommand(std::string &command) {
    int index = -1;
    for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
        if (command == availableCommands[i]) {
            index = i;
        }
    }
    return index;
}

bool quit() {
	std::cout << "Exiting\n";
	return false;
}

char* getCommandsAsChar(const std::string &command, int expectedCommands, int nrOfCommands)
{
	char * folderpath = nullptr;
	if (nrOfCommands == expectedCommands)
	{
		folderpath = new char[command.length() + 1];
		for (unsigned int i = 0; i < command.length() + 1; i++)
		{
			folderpath[i] = command[i];
		}
	}
	return folderpath;
}

std::string help() {
    std::string helpStr;
    helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
    helpStr += "-----------------------------------------------------------------------------------\n" ;
    helpStr += "* quit:                             Quit OSD Disk Tool\n";
    helpStr += "* format;                           Formats disk\n";
    helpStr += "* ls     <path>:                    Lists contents of <path>.\n";
    helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
    helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
    helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
    helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
    helpStr += "* rm     <file>:                    Removes <file>\n";
    helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
    helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
    helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
    helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";
    helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";
    helpStr += "* pwd:                              Get current working directory\n";
	helpStr += "* clear:                            Clears the terminal window\n";
	helpStr += "* tree:                             Lists contents of directories in a tree-like format\n";
	helpStr += "* chmod <+/-rw> <file/directory>    Change permission of a directory or file\n";
    helpStr += "* help:                             Prints this help screen\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */

bool endCommandFound(const std::string &str)
{
	bool returnValue = false;
	if (str.length() > 3)
		returnValue = str[str.length() - 4] == ':' && str[str.length() - 3] == 'w' && str[str.length() - 2] == 'q';
	return returnValue;
}

std::string advancedEditor()
{
	std::string content = "";
	std::string adder = "";

	clearScr();
	std::cout << "\t\t\t\t\tAdvanced Editor - type \":wq\" to save and exit\n";
	while (content.length() < 3 || !endCommandFound(content))
	{
		std::getline(std::cin, adder);
		content += adder;
		content += '\n';
	}

	for (int i = 0; i < 4; i++)
		content.pop_back();

	content += '\0';
	clearScr();
	return content;
}
