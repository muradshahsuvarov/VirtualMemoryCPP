#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "Consts.h"

#pragma once
class MainMemory
{

private:
	MainMemory* nextMainMemoryCell;
	char* dataBuffer;
	int physicallAddress;
	

	

	public : void setNextMainMemoryCell(MainMemory* _next) {
		nextMainMemoryCell = _next;
	}

	public: MainMemory* getNextMainMemoryCell() {
		return nextMainMemoryCell;
	}

	public : void setDataBuffer(char* _dataBuffer) {
		dataBuffer = _dataBuffer;
	}
    
	public: char* getDataBuffer() {
		return dataBuffer;
	}

	public: void setPhysicalAddress(int _physicalAddress) {
		physicallAddress = _physicalAddress;
	}

	public: int getPhysicalAddress() {
		return physicallAddress;
	}

	public: void resetNextMemoryCell() {
		nextMainMemoryCell = new MainMemory();
	}

};


bool pageInPageTable(int page_number);
int frameNumberBackStore(int page_number, char value[], int page_offset);
int getPageUsingLRU(int TLB_Entry, int Track[], int TLB[TLB_SIZE][2], int PAGE[PAGE_SIZE][2], int LRU_Index, int page_number);
int PutInMainMemory(MainMemory* head, char buffer[], int page_number);
bool readFromDisk(int _pageNumber, char _buffer[]);
void StartSimulation();
bool file_is_empty(std::ifstream& pFile);
int frameNumberBackStore(int page_number, char value[], int page_offset);
int PutInMainMemory(MainMemory* head, char buffer[], int page_number);
bool readFromDisk(int _pageNumber, char _buffer[]);
int getPageUsingLRU(int TLB_Entry, int Track[], int TLB[TLB_SIZE][2], int PAGE[PAGE_SIZE][2], int LRU_Index, int page_number);
void WriteIntroIntoFile();
