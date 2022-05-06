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
int pagehit_counter = 0, pagefault_counter = 0;
int page_stack[2000];
int tlb_index = 0;
int tlb_hit_index = 0, lru_index = 0;
int frame_number = 0;
int tlb_hit_flag = 0, tlb_entry = 0;
int tlb[TLB_SIZE][2];
int virtualge_physical_page[PAGE_SIZE][2];
int succ_transl_counter = 0, test = 0;
char value[256];

bool pageInPageTable(int page_number, int PAGE[PAGE_SIZE][2])
{
    if (PAGE[page_number][1] == 1) {  return true; }
    else { return false; }
}

bool file_is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

int frameNumberBuffer(int page_number, char value[], int page_offset)
{
    
    char buffer[256] = {NULL};
    int FrameNumber = 0;
    
    MainMemory* list = new MainMemory();
    list->setNextMainMemoryCell(NULL);
    list->setDataBuffer(NULL);

    bool data_is_read = readFromDisk(page_number, buffer);

    FrameNumber = PutInMainMemory(list, buffer, page_number);
    value[page_number] = buffer[page_offset];

    return FrameNumber;
}

int PutInMainMemory(MainMemory* head, char buffer[], int page_number)
{

    MainMemory* current = head;
    while (current->getNextMainMemoryCell() != NULL)
    {
        current = current->getNextMainMemoryCell();
    }

    /* Adding Physicall Address */

    current->setDataBuffer(buffer);
    current->setPhysicalAddress(rand() % 1000 + 1000 + page_number);
    current->resetNextMemoryCell();
    current->getNextMainMemoryCell()->setDataBuffer(NULL);
    current->getNextMainMemoryCell()->setNextMainMemoryCell(NULL);

    /* Adding Physicall Address */

    return current->getPhysicalAddress();

}
bool readFromDisk(int _pageNumber, char _buffer[])
{
    std::ifstream fp("disk_sim", std::fstream::in | std::fstream::out | std::fstream::app);


    if (file_is_empty(fp)) {
        printf("read BACKING_STORE wrong\n");
    }

    fp.seekg(_pageNumber * 256, SEEK_SET);
    fp.read(_buffer, 256);

    fp.close();

    return false;
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

void StartSimulation()
{
    WriteIntroIntoFile();

    int i = 0;
    //initizate the TLB table
    for (i = 0; i < TLB_SIZE; i++)
    {
        tlb[i][0] = -1;// Page is null
        tlb[i][1] = -1;// Frame number is null
    }
    //initizate the Page table
    for (i = 0; i < PAGE_SIZE; i++)
    {
        virtualge_physical_page[i][0] = -1;//-1 means no frame number
        virtualge_physical_page[i][1] = -1;//1 is valid; -1 is invalid
    }

    std::ifstream fp("addresses.txt");
    std::string line;
    size_t len = 0;
    size_t read;


    for (; std::getline(fp, line); )
    {

        int virtual_address = std::stoi(line);

        int page_offset = std::stoi(line) & 255;
        int page = std::stoi(line) & 65280;
        int page_number = page >> 8;

        // Check whether LRU contains a virtual address or not
        for (i = 0; i < TLB_SIZE; i++)
        {
            
            if (tlb[i][0] == page_number)
            {
                tlb_hit_flag = 1;
                tlb_hit_index = i;
                break;
            }
        }

        // Check whether TLB hit occurred or not
        if (tlb_hit_flag == 1) // TLB HIT occurred 
        {
            tlb_hit_counter++;
            //TLB HIT occurred: Get the frame number and translate it into the physical address
            printf("Virtual address: %d          Physical address: %d          Value: %hhd\n", virtual_address, (tlb[tlb_hit_index][1] * 256 * 10 + page_offset), value[page_number]);
            std::stringstream _text_report;
            _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << (tlb[tlb_hit_index][1] * 256 * 10 + page_offset) << "          " << "Value: " << (int)value[page_number];
            WriteToFile(_text_report.str());
            tlb_hit_flag = -1;
            tlb_hit_index = -1;
        }
        else
        {
            tlb_miss_counter++;
            if (pageInPageTable(page_number, virtualge_physical_page)) {

                pagehit_counter++;       
                tlb_entry = getPageUsingLRU(tlb_entry, page_stack, tlb, virtualge_physical_page, lru_index, page_number);
                std::stringstream _text_report;
                _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << (virtualge_physical_page[page_number][0] * 256 + page_offset) << "          " << "Value: " << (int)value[page_number];
                WriteToFile(_text_report.str());
            }
            else
            {
               
                frame_number = frameNumberBuffer(page_number, value, page_offset);
                virtualge_physical_page[page_number][0] = frame_number;
                virtualge_physical_page[page_number][1] = 1;
                tlb_entry = getPageUsingLRU(tlb_entry, page_stack, tlb, virtualge_physical_page, lru_index, page_number);
                std::stringstream _text_report;
                _text_report << "Virtual address: " << virtual_address << "          " << " Physical address: " << frame_number * 256 + page_offset << "          " << "Value: " << (int)value[page_number];
                WriteToFile(_text_report.str());
                pagefault_counter++;
            }
        }
        ++succ_transl_counter;

    }


    // Metrics
    printf("Translated addresses: %d\n", succ_transl_counter);
    printf("Page fault rate: %f%\n", (((float)pagefault_counter / (float)(pagefault_counter + pagehit_counter + tlb_hit_counter)) * 100));
    printf("TLB hit rate: %f%\n", (((float)tlb_hit_counter / (float)(tlb_hit_counter + tlb_miss_counter)) * 100));


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
            _lru_index = (_tlb_entry - 16) % 16;
            _tlb[_lru_index][0] = _page_number;
            _tlb_entry++;

        }


    }else {
        _tlb[_tlb_entry][0] = _page_number;
        _tlb[_tlb_entry][1] = _page[_page_number][0];
        _page_stack[_tlb_entry] = _page_number;
        _tlb_entry++;
    }


    return _tlb_entry;

}

