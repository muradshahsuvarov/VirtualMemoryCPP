#include "MainMemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "MainMemory.h"
#include <time.h>
#include <fstream>
#include <string>
#include <sstream>


using namespace std;

int tlb_hit_counter = 0, tlb_miss_counter = 0;
int pagehit_counter = 0, tlb_hit_index = 0, pagefault_counter = 0;
int page_stack[4050];
int tlb_index = 0, lru_index = 0;
int frame_number = 0;
int tlb_hit_flag = 0, tlb_entry = 0, tlb_hit_freq;
int tlb[TLB_SIZE][2];
int physical_page[PAGE_SIZE][2];
int succ_transl_counter = 0, test = 0;
char value[256];

bool pageInPageTable(int page_number)
{
    if (physical_page[page_number][1] == 1) {  return true; }
    else { return false; }
}

bool file_is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

int frameNumberBackStore(int _page_number, char _value[], int _page_offset)
{
    // Initialize the buffer which will be used for reading from the disk
    char char_buffer[256] = {NULL};
    // Initialize the temporary frame number to 0
    int FrameNumber = 0;
    
    // Initialize the Main Memory
    MainMemory* list = new MainMemory();
    list->setNextMainMemoryCell(NULL);
    list->setDataBuffer(NULL);

    // Read the data from the disk into the char_buffer
    bool data_is_read = readFromDisk(_page_number, char_buffer);

    if (data_is_read) {
        
        // chat_buffer is needed for finding the virtual address assositated value from the disk
        // If data has been retrieved from the binary without exceptions, then put it (char_buffer) into the main memory
        FrameNumber = PutInMainMemory(list, char_buffer, _page_number);
        // Assinging the value to the virtual page
        _value[_page_number] = char_buffer[_page_offset];

        return FrameNumber;
    }
    
    return 0;
    
}


int PutInMainMemory(MainMemory* head, char buffer[], int page_number)
{

    MainMemory* current_memory_cell = head;
    // Get the last main memory cell
    while (current_memory_cell->getNextMainMemoryCell() != NULL)
    {
        current_memory_cell = current_memory_cell->getNextMainMemoryCell();
    }

    // Set the next memory cell to point to null
    current_memory_cell->resetNextMemoryCell();
    current_memory_cell->getNextMainMemoryCell()->setDataBuffer(NULL);
    current_memory_cell->getNextMainMemoryCell()->setNextMainMemoryCell(NULL);

    /* Adding Physicall Address */

    current_memory_cell->setDataBuffer(buffer); // Put already found character buffer into the last maun memory cell
    if (page_number > 5000)
        current_memory_cell->setPhysicalAddress( (page_number / 256 ) * 10000); // Generate the physical address
    else
        current_memory_cell->setPhysicalAddress(page_number * 256); // Generate the physical address
    

    /* Adding Physicall Address */

    return current_memory_cell->getPhysicalAddress();

}
// Reading bits from the disk file
bool readFromDisk(int _pageNumber, char _buffer[])
{
    try {
        // Read the disk_sim binary file
        std::ifstream fp("disk_sim", std::fstream::in | std::fstream::out | std::fstream::app);

        fp.seekg(_pageNumber * 256, SEEK_SET);
        fp.read(_buffer, 256);


        fp.close();
    }
    catch (exception _e) {
        return false;
    }
    return true;
}


void WriteToFile(string _text) {

    ofstream myfile;
    myfile.open("logs.txt", std::ios_base::app);
    myfile << _text << "\n";
    myfile.close();

}


void WriteIntroIntoFile() {
    ofstream myfile;
    myfile.open("logs.txt");
    myfile << "Welcome to Team X's Virtual Memory Simulator!";
    myfile << "\n\nSystem parameters:";
    myfile << "\n\nProgram address space: 16-bit";
    myfile << "\nPage size: 2^8 bytes";
    myfile << "\nTLB capacity: 16 entries";
    myfile << "\nNumber of frames: 256";
    myfile << "\nPhysical memory size: 64 KB";
    myfile << "\nReplacement algorithm: FIFO";
    myfile << "\n\n-----------------------------------------------------------------------------------";
    myfile << "\n\nStarting...\n\n\n";
    myfile.close();

}

void WriteMetricsIntoFile(int _pagefault_counter,
    int _succ_transl_counter,
    int _tlbhit_counter,
    int _tlbmiss_counter) {

    ofstream myfile;
    myfile.open("logs.txt", std::ios_base::app);
    myfile << "\n\nFinished";
    myfile << "\n\n-----------------------------------------------------------------------------------";
    myfile << "\n\nStatistics:";
    myfile << "\n\nTranslated addresses: " << _succ_transl_counter;
    myfile << "\n\nPage fault rate: " << ((float)pagefault_counter / (float)(pagefault_counter + pagehit_counter + tlb_hit_counter)) * 100 << "%";
    myfile << "\n\nTLB hit rate: " << ((float)tlb_hit_counter / (float)(tlb_hit_counter + tlb_miss_counter)) * 100 << "%";
    myfile.close();

}

    void InitializeTLB() {

        // TLB is a 16 X 2 matrix (2D Array) comprising of pages and the relevant frames. 
        // The shape of the matrix is 16 X 2 , because the TLB size is 16 as per the homework
        // requirenments, 2 is devoted for storing the page and frame on each 1 - 16 slot.
        // First we initialize the tlb with -1s. When a slot is -1, it is empty, so no
        // page (Virtual Memory) or frame (Physical Memory) is stored.
        // | -1 | -1 | - 0
        // | -1 | -1 | - 1
        //  .........
        // | -1 | -1 | - 15
        for (int i = 0; i < TLB_SIZE; i++)
        {
            tlb[i][0] = -1; // Page is null
            tlb[i][1] = -1; // Frame number is null
        }

    }

    void InitializePageTable() {

        // Page Table is a 256 X 2 matrix comprising of the page and the relevant page status.
        // The first column (matrix cell) is a frame number, the second is status (BIT STATUS VALID, INVALID = DIRTY).
        // Initially all the frames are empty with an invalid bit.
        // Valid bit means that the page is in the Page Table
        for (int i = 0; i < PAGE_SIZE; i++)
        {
            physical_page[i][0] = -1; // Frame number is null.
            physical_page[i][1] = -1; // 1 = valid, -1 = invalid
        }

    }


    string decimalToBinary(int n)
    {
        string output = "";
        // Size of an integer is assumed to be 32 bits
        for (int i = 15; i >= 0; i--) {
            int k = n >> i;
            if (k & 1)
                output += "1";
            else
                output += "0";
        }

        return output;
    }

    int binaryToDecimal(string n)
    {
        string num = n;
        int dec_value = 0;

        // Initializing base value to 1, i.e 2^0
        int base = 1;

        int len = num.length();
        for (int i = len - 1; i >= 0; i--) {
            if (num[i] == '1')
                dec_value += base;
            base = base * 2;
        }

        return dec_value;
    }


void StartSimulation()
{
    // Writing the header text into the logs.txt file
    WriteIntroIntoFile();

    // Initializing the Table Lookaside Buffer (Page Table Cache)
    InitializeTLB();
    
    // Initializing the Page Table
    InitializePageTable();


    // File Reader
    std::ifstream fp("addresses.txt");
    std::string line;
    size_t len = 0;
    size_t read;


    // Iterating line by line through the addresses.txt
    for (;std::getline(fp, line);)
    {
        // Converting the input virtual address to binary for the further processing
        int virtual_address = std::stoi(line); // Take out number from address.txt and convert it into int
        string virtual_address_binary = decimalToBinary(virtual_address); // convert the number into 16 bit binary number
        string virtual_address_binary_page = virtual_address_binary.substr(0, 8); // we read the bytes from left to 8 bits right 
        string virtual_address_binary_offset = virtual_address_binary.substr(8, VIRTUAL_ADDRESS_SIZE); // read last 8 bits from the 8 bit of the binary converted virtual address

        // Retreiving the page offset and page number from the virtual address
        // Page Number: 00000010 = 2 Page Offset: 00000001 = 1
        int page_offset = binaryToDecimal(virtual_address_binary_offset); // Bitwise operation on line and 255.
        int page_number = binaryToDecimal(virtual_address_binary_page); // Retrieving first 8 bits from the page line

        // Check whether TLB contains a virtual address (page number) or not
        for (int i = 0; i < TLB_SIZE; i++)
        {
            if (tlb[i][0] == page_number)
            {
                // Page is found in the TLB, so the TLB Hit occurred
                tlb_hit_flag = 1; // Shows that the hit occurred
                tlb_hit_index = i; // Shows the place in the TLB where the page hit occurred
                break;
            }
        }

        // TLB HIT occurred 
        if (tlb_hit_flag == 1)
        {
            // TLB HIT occurred: Increment the TLB Hit counter for the further metrics calculations
            tlb_hit_counter++;
            // TLB HIT occurred: Get the frame number
            std::stringstream _text_report;
            _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << (tlb[tlb_hit_index][1]) << "          " << "Value: " << (int)value[page_number];
            WriteToFile(_text_report.str());

            // Setting back the tlb_hit_flag and tlb_hit_index to -1
            tlb_hit_flag = -1;
            tlb_hit_index = -1;
        }
        else // TLB MISS occurred , Check the page in the Page Table

        {
            // Incrementing the tlb_miss_counter for the futher metrics
            tlb_miss_counter++;

            // Check if the page is in the page table
            if (pageInPageTable(page_number)) {

                // Increment the pagehit_counter for the further page hit metrics
                pagehit_counter++;       
                // Get the tlb_entry using the LRU algorithm
                tlb_entry = getPageUsingLRU(tlb_entry, page_stack, tlb, physical_page, lru_index, page_number);
                std::stringstream _text_report;
                _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << (physical_page[page_number][0]) << "          " << "Value: " << (int)value[page_number];
                WriteToFile(_text_report.str());
            }
            else
            {
                // If the page number is not in the Page Table , then find it from the buffer
                frame_number = frameNumberBackStore(page_number, value, page_offset);
                // Map the already found frame number to the page number in physical page (Page Table) 
                physical_page[page_number][0] = frame_number;
                // Set the bit of the found page number - frame number to valid = 1
                physical_page[page_number][1] = 1;
                tlb_entry = getPageUsingLRU(tlb_entry, page_stack, tlb, physical_page, lru_index, page_number);
                std::stringstream _text_report;
                _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << frame_number * 256 + page_offset << "          " << "Value: " << (int)value[page_number];
                WriteToFile(_text_report.str());
                pagefault_counter++;
            }
        }
        ++succ_transl_counter;

    }


    // Metrics
    printf("The results were saved in logs.txt");


    WriteMetricsIntoFile(pagefault_counter, succ_transl_counter, tlb_hit_counter, tlb_miss_counter);

    getchar();
}




int getPageUsingLRU(int _tlb_entry, int _page_stack[], int _tlb[TLB_SIZE][2], int _page[PAGE_SIZE][2], int _lru_index, int _page_number)
{
    int iterator = 0, flag = 0;

    if (TLB_SIZE <=  _tlb_entry) {

        bool stop_iterating = false;
        iterator = _tlb_entry - 1;

        while (!stop_iterating && iterator >= _tlb_entry - 16) {

            if (_page_stack[iterator] == _page_number) {
                flag = 1;
                stop_iterating = true;
            }
            iterator--;
        }


        if (flag == 0)
        {
            // Optimizing the TLB entries
            _lru_index = (_tlb_entry - 16) % 16;

            // Update the tlb
            _tlb[_lru_index][0] = _page_number;

            _tlb_entry++;

        }


    }else {
        // Update the tlb 
        _tlb[_tlb_entry][0] = _page_number;
        _tlb[_tlb_entry][1] = _page[_page_number][0];
        _page_stack[_tlb_entry] = _page_number;
        _tlb_entry++;
    }


    return _tlb_entry;

}


void main()
{
    // Starting the virtual memory map simulation
    StartSimulation();
}

