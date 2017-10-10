
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

	char * target = nullptr;
	char * destination = nullptr;
	std::string content;
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
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (fs.listCopy(target, content))
					std::cout << content << std::endl;
				else
					std::cout << "ls: cannot access '" << target << "': No such file or directory\n";
				content = "";
				delete[] target;
                break;
            case 3: // create E
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target)
				{
					clearScr();
					std::cout << "\t\t\t\t\t\tAdvanced Editor\n";
					std::string content;
					std::getline(std::cin, content);
					content += '\0';
					fs.createFile(target, content.c_str(), content.length());
					clearScr();
				}
				else
				{
					std::cout << "createImage <folderpath>\n";
				}
				
                break;
            case 4: // cat E
				//std::cout << fs.lol();
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target)
				{
					std::string fileContent = fs.getFileContent(target);
					delete[] target;
					if (fileContent != "")
					{
						std::cout << fileContent << std::endl;
					}
					else
					{
						std::cout << target << " is a directory\n";
					}
				}
				else
				{
					std::cout << "cat <filepath>" << std::endl;
				}

                break;
            case 5: // createImage E DONE
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target)
				{
					fs.createImage(target);
					delete[] target;
				}
				else
				{
					std::cout << "createImage <folderpath>\n";
				}
                break;
            case 6: // restoreImage E DONE
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target)
				{
					if (!fs.readImage(target))
					{
						std::cout << "restoreImage: cannot read '" << target << "': No such file\n";
					}
					delete[] target;
				}
				else
				{
					std::cout << "restoreImage <folderpath>\n";
				}
                break;
            case 7: // rm E
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target)
				{
					if (!fs.removeFolder(target))
					{
						std::cout << "rm: cannot access '" << target << "': No such file or directory\n";
					}
					delete[] target;
				}
				else
				{
					std::cout << "rm <path>\n";
				}
                break;
            case 8: // cp E DONE
				target = getCommandsAsChar(commandArr[1], 3, nrOfCommands);
				destination = getCommandsAsChar(commandArr[2], 3, nrOfCommands);
				if (target && destination)
				{
					if (!fs.copyTarget(target, destination))
					{
						std::cout << "cp: cannot access '" << target << " or " << destination << "': No such file or directory\n";
					}
					delete[] target;
					delete[] destination;
				}
				else
				{
					std::cout << "cp <target> <destination>\n";
				}
                break;
            case 9: // append
                break;
            case 10: // mv
                break;
            case 11: // mkdir E	DONE		
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target != nullptr)
				{
					if (!fs.createFolder(target))
						std::cout << "mkdir: cannot create directory '" << target << "': File exists\n";
					delete[] target;
				}
				else
				{
					std::cout << "mkdir <folderpath>" << std::endl;
				}
				
                break;
            case 12: // cd E DONE
				target = getCommandsAsChar(commandArr[1], 2, nrOfCommands);
				if (target != nullptr)
				{
					if (!fs.changeDir(target))
						std::cout << "bash: cd: " << target << ": No such file or directory\n";
					delete[] target;
					currentDir = fs.currentDir();
				}
				else
				{
					std::cout << "cd <filepath>" << std::endl;
				}
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
