#include <iostream>
#include <fstream>
#include <string>
using namespace std;

const int PAGE_NUM = 128;
const int PAGE_SIZE = 32;

const int SECTOR_NUM = 128;
const int SECTOR_SIZE = 32;

const int PDBR = 0xd80;

unsigned char memory[PAGE_NUM * PAGE_SIZE];
unsigned char disk[SECTOR_NUM * SECTOR_SIZE];

void outputMem()
{
	for (int i = 0; i < PAGE_NUM; ++i){
		for (int j = 0; j < PAGE_SIZE; ++j){
			cout << hex << (int)memory[i * 128 + j] << " ";
		}
		cout << endl;
	}
}

int main()
{
	ifstream fin("data.txt");
	if (!fin)
		cout << "failed to open file" << endl;

	string s, num_s;
	int temp, base;
	//input memory data
	for (int i = 0; i < PAGE_NUM; ++i){
		fin >> s >> num_s;
		base = i * PAGE_SIZE;
		for (int j = 0; j < PAGE_SIZE; ++j){
			fin >> hex >> temp;
			memory[base + j] = temp;
		}
	}
	//input disk data
	for (int i = 0; i < SECTOR_NUM; ++i){
		fin >> s >> num_s;
		base = i * SECTOR_SIZE;
		for (int j = 0; j < SECTOR_SIZE; ++j){
			fin >> hex >> temp;
			disk[base + j] = temp;
		}
	}

	cout << "Virtual Address: ";
	int virtual_addr;
	cin >> hex >> virtual_addr;

	int pde_index, pte_index, offset;
	offset = virtual_addr % 32;
	virtual_addr /= 32;
	pte_index = virtual_addr % 32;
	virtual_addr /= 32;
	pde_index = virtual_addr % 32;

	unsigned char pde = memory[PDBR + pde_index];
	int pt_base = pde & 127;
	int pde_valid = pde / 128;
	cout << "--> pde index:0x" << hex << pde_index << " pde contents:(valid " << pde_valid << ", pfn 0x" << hex << pt_base << ")" << endl;
	
	unsigned char pte;
	if (pde_valid) 
		pte = memory[PAGE_SIZE * pt_base + pte_index];
	else if (pte_index != 0x7f)
		pte = disk[SECTOR_SIZE * pt_base + pte_index];
	else{
		cout << "        " << "--> Fault (pte not found)" << endl;
		return 0;
	}
	int page_num = pte & 127;
	int pte_valid = pte / 128;
	cout << "    " << "--> pte index:0x" << hex << pte_index << " pte contents:(valid " << pte_valid << ", pfn 0x" << hex << page_num << ")" << endl;

	int physical_addr = page_num * 32 + offset;
	if(pte_valid)
		cout << "        " << "--> To Physical Address 0x" << hex << physical_addr << " --> Value: 0x" << hex << (int)memory[physical_addr] << endl;
	else if(page_num != 0x7f)
		cout << "        " << "--> To Disk Sector Address 0x" << hex << physical_addr << " --> Value: 0x" << hex << (int)disk[physical_addr] << endl;
	else
		cout << "        " << "--> Fault (value not found)" << endl;
	return 0;
}






