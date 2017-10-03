
#include <iostream>
#include <sstream>
#include<string>
#include "filesystem.h"

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 16;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help","clear"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
char* getCommandsAsChar(const std::string &command, int expectedCommands, int nrOfCommands);
void clearScr();

std::string help();

/* More functions ... */

int main(void) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	FileSystem fs;
	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "Chefen@Computer";    // Change this if you want another user to be displayed
	std::string currentDir = fs.currentDir();    // current directory, used for output
    bool bRun = true;

	char * folderpath = nullptr;
	std::string content;
    do {
        std::cout << user << ":" << fs.pwd() << "$ ";
		getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {

            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

			case 0: //quit E
				bRun = quit();                
                break;
            case 1: // format E
				fs.formatHDD();
                break;
            case 2: // ls E
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (fs.listCopy(folderpath, content))
					std::cout << content << std::endl;
				else
					std::cout << "ls: cannot access '" << folderpath << "': No such file or directory\n";
				content = "";
				delete[] folderpath;
                break;
            case 3: // create E
				//std::cout << fs.lol();
				// 1. Skriver create <filename>.
				// 2. Editor öppnas
				// 3. Skickar in filename och content
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath)
				{
					clearScr();
					std::cout << "\t\t\t\t\t\tAdvanced Editor\n";
					std::string content;
					std::getline(std::cin, content);
					fs.createFile(folderpath, content.c_str(), content.length());
					//delete[] folderpath;
				}
				else
				{
					std::cout << "createImage <folderpath>\n";
				}
				
                break;
            case 4: // cat E
				std::cout << fs.lol();
                break;
            case 5: // createImage E
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath)
				{
					fs.createImage(folderpath);
					delete[] folderpath;
				}
				else
				{
					std::cout << "createImage <folderpath>\n";
				}
                break;
            case 6: // restoreImage E
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath)
				{
					fs.readImage(folderpath);
					delete[] folderpath;
				}
				else
				{
					std::cout << "restoreImage <folderpath>\n";
				}
                break;
            case 7: // rm E
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath)
				{
					if (!fs.removeFolder(folderpath))
					{
						std::cout << "rm: cannot access '" << folderpath << "': No such file or directory\n";
					}
					delete[] folderpath;
				}
				else
				{
					std::cout << "rm <path>\n";
				}
                break;
            case 8: // cp E
                break;
            case 9: // append
                break;
            case 10: // mv
                break;
            case 11: // mkdir E
				
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath != nullptr)
				{
					if (!fs.createFolder(folderpath))
						std::cout << "mkdir: cannot create directory '" << folderpath << "': File exists\n";
					delete[] folderpath;
				}
				else
				{
					std::cout << "mkdir <folderpath>" << std::endl;
				}
				
                break;
            case 12: // cd E
				folderpath = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (folderpath != nullptr)
				{
					if (!fs.changeDir(folderpath))
						std::cout << "bash: cd: " << folderpath << ": No such file or directory\n";
					delete[] folderpath;
					currentDir = fs.currentDir();
				}
				else
				{
					std::cout << "cd <filepath>" << std::endl;
				}
                break; 
            case 13: // pwd E
				std::cout << fs.pwd() << std::endl;
                break;
            case 14: // help
                std::cout << help() << std::endl;
                break;
			case 15: // clear
				clearScr();
				break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
        }
    } while (bRun == true);

    return 0;
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
		for (int i = 0; i < command.length() + 1; i++)
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
    helpStr += "* help:                             Prints this help screen\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */
