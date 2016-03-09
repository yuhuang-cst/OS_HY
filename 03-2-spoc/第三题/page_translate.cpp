#include <iostream>
#include <fstream>
#include <string>
using namespace std;

const int PAGE_NUM = 128;
const int PAGE_SIZE = 32;
const int PDBR = 544;

unsigned char memory[PAGE_NUM * PAGE_SIZE];

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
	string page_s, page_num_s;
	int temp;
	for (int i = 0; i < PAGE_NUM; ++i){
		fin >> page_s >> page_num_s;
		int base = i * PAGE_SIZE;
		for (int j = 0; j < PAGE_SIZE; ++j){
			fin >> hex >> temp;
			memory[base + j] = temp;
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
	if (pde / 128)
		cout << "--> pde index:0x" << hex << pde_index << " pde contents:(valid " << 1 << ", pfn 0x" << hex << pt_base << ")" << endl;
	else{
		cout << "Fault (page directory entry not valid)" << endl;
		return 0;
	}

	unsigned char pte = memory[PAGE_SIZE * pt_base + pte_index];
	int physical_page = pte & 127;
	if (pte / 128)
		cout << "    " << "--> pte index:0x" << hex << pte_index << " pte contents:(valid " << 1 << ", pfn 0x" << hex << physical_page << ")" << endl;
	else{
		cout << "Fault (page table entry not valid)" << endl;
		return 0;
	}

	int physical_addr = physical_page * 32 + offset;
	cout << "        " << "--> Translates to Physical Address 0x" << hex << physical_addr << " --> Value: 0x" << hex << (int)memory[physical_addr] << endl;


	return 0;
}






