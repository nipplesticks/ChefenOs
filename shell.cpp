#include <iostream>
#include <sstream>
#include<string>
#include "filesystem.h"

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 15;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
char* getCommandsAsChar(const std::string &command, int expectedCommands, int nrOfCommands);


std::string help();

/* More functions ... */

int main(void) {

	FileSystem fs;
	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "Chefen@Computer";    // Change this if you want another user to be displayed
	std::string currentDir = fs.currentDir();    // current directory, used for output
    bool bRun = true;

	char * folderpath = nullptr;

    do {
        std::cout << user << ":" << currentDir << "$ ";
        getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {

            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

			case 0: //quit E
				bRun = quit();                
                break;
            case 1: // format E
                break;
            case 2: // ls E
				std::cout << fs.listDir();
                break;
            case 3: // create E
                break;
            case 4: // cat E
                break;
            case 5: // createImage E
                break;
            case 6: // restoreImage E
                break;
            case 7: // rm E
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
					fs.createFolder(folderpath);
					delete[] folderpath;
				}
				else
				{
					std::cout << "mkdir <filepath>" << std::endl;
				}
                break;
            case 12: // cd E
                break; 
            case 13: // pwd E
                break;
            case 14: // help
                std::cout << help() << std::endl;
                break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
        }
    } while (bRun == true);

    return 0;
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
    helpStr += "* help:                             Prints this help screen\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */
