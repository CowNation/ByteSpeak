#include <iostream>
#include <string.h>
#include <fstream>
#include <regex>

// this will have the biggest impact on memory. for each line, a LINE_DATA_BUFFER_SZ number of bytes will be initialized for that line's buffer. set this to the maximum amount of bytes you think you'll need to cache for a line
#define LINE_DATA_BUFFER_SZ 64

typedef unsigned char byte;

byte** lineDataBuffer = nullptr;

const std::regex lineRegex = std::regex(R"(obuf<-(\[.*?\])+|(\[.*?\])+)");
const std::regex byteRegex = std::regex(R"(\[U*(1?[0-9]{1,2}|2[0-4][0-9]|25[0-5]\b)(->\d*)?\])");

int currentLine = 0;

byte getByteValue(std::string byteText) {
	if (!byteText.length())
		return 0;

	if (byteText[0] == 'U'){
		int lineIndex = currentLine - std::count(byteText.begin(), byteText.end(), 'U');
		int byteIndex = std::stoi(byteText.substr(byteText.find_last_of("U") + 1));
		return lineDataBuffer[lineIndex][byteIndex];
	}
	else
		return (byte)std::stoi(byteText);
}

void executeFile(std::string fileName) {
	std::ofstream outFile;
	outFile.open("output.asm");
	outFile << "section .text" << std::endl;
	outFile << "	global _start ; tells linker the entry point" << std::endl; 
	outFile << "_start:" << std::endl;

	std::ifstream file;
	file.open(fileName);

	int numLines = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n') + 1;
	
	// initializes a data buffer for each line
	lineDataBuffer = new byte*[numLines];

	// clear the file stream back to the beginning
	file.clear();
	file.seekg(0, std::ios::beg);

	currentLine = 0;
	while (!file.eof()) {
		// initialize the data buffer for this line
		lineDataBuffer[currentLine] = new byte[LINE_DATA_BUFFER_SZ];
		memset(lineDataBuffer[currentLine], 0, LINE_DATA_BUFFER_SZ);

		std::string line;
		std::getline(file, line);

		// remove all whitespace from this line
		// line.erase(remove_if(line.begin(), line.end(), isspace), line.end());

		std::smatch lineRgx;
		if (!std::regex_match(line, lineRgx, lineRegex)) {
			std::cout << std::to_string(currentLine + 1) << ") Invalid line format\n";
			throw;
		}

		// this should just be all of the bytes
		std::string bytesText = line.substr(line.find("["));
		std::smatch bytesRgx;

		// the bytes will be immediately outputted
		bool outputting = line.find("obuf") == 0;
		// the bytes will be cached into the line data buffer
		bool caching = !outputting;

		int currentByte = 0;
		while (std::regex_search(bytesText, bytesRgx, byteRegex)) { 
			std::string byteText = bytesRgx.str(0);

			// remove the surrounding brackets []
			byteText = byteText.substr(1, byteText.length() - 2);

			int goBackOperator = byteText.find("->");
			if (goBackOperator != -1) {
				// if there are no U's in the byte, the target line was not specified ([156->] passes through the regex pattern so we must check )
				if (byteText.find('U') == -1){
					std::cout << std::to_string(currentLine + 1) << ") Cannot use the go back operator (->), the target line was not specified\n";
					throw;
				}

				int lengthLimit = goBackOperator + 2 < byteText.length() ? std::stoi(byteText.substr(goBackOperator + 2)) : -1;

				// remove the -> and anything after it from the end of the byte text
				byteText = byteText.substr(0, goBackOperator);

				byte* targetLine = lineDataBuffer[currentLine - std::count(byteText.begin(), byteText.end(), 'U')];
				int seekingByteIndex = std::stoi(byteText.substr(byteText.find_last_of("U") + 1));

				int numSeeked = 0;
				byte seekingByte = targetLine[seekingByteIndex];
				while (seekingByte) {
					// only seek a lengthLimit number of bytes
					if (numSeeked == lengthLimit)
						break;

					lineDataBuffer[currentLine][currentByte] = seekingByte;

					seekingByteIndex++;
					seekingByte = targetLine[seekingByteIndex];
					numSeeked++;

					currentByte++;
				}
			}
			else {
				byte byteValue = getByteValue(byteText);
				lineDataBuffer[currentLine][currentByte] = byteValue;

				currentByte++;
			}

			// suffix to find the rest of the string
			bytesText = bytesRgx.suffix().str();
		}

		if (currentByte == 0) {
			std::cout << std::to_string(currentLine + 1) << ") Invalid byte format " << bytesText << std::endl;
			throw;
		}

		if (outputting) {
			outFile << "	mov	ecx, LINE_" << std::to_string(currentLine + 1) << std::endl;
			outFile << "	mov	edx, " << std::to_string(currentByte) << std::endl;
			outFile << "	call print" << std::endl << std::endl;
		}

		currentLine++;
	}

	outFile << "	mov	eax, 1 ; sys_exit" << std::endl;
	outFile << "	int	0x80" << std::endl;

	outFile << "print:" << std::endl;
	outFile << "	mov	ebx, 1 ; file descriptor (stdout)" << std::endl;
	outFile << "	mov	eax, 4 ; system call number (sys_write)" << std::endl;
	outFile << "	int	0x80 ; call kernel" << std::endl;
	outFile << "	ret" << std::endl << std::endl;

	outFile << "section .data" << std::endl;

	for (int i = 0; i < numLines; i++){
		outFile << "	LINE_" << std::to_string(i + 1) << " db ";
		for (int j = 0; lineDataBuffer[i][j]; j++) {
			if (j > 0)
				outFile << ", ";
		
			outFile << std::to_string(lineDataBuffer[i][j]);
		}

		outFile << std::endl;
	}
}

int main() {
	executeFile("test.bs");
}