/*
 * rampolska.h
 *
 *  Created on: Nov 11, 2015
 *      Author: rkouere
 */

#ifndef RAM_H_
#define RAM_H_
#include "systemc.h"

static const int MEM_SIZE = 512;
SC_MODULE(Memory) {
public:
	enum Function {
		FUNC_READ, FUNC_WRITE
	};
	enum RetCode {
		RET_READ_DONE, RET_WRITE_DONE,
	};
	sc_in<bool> Port_CLK;
	sc_in<Function> Port_Func;
	sc_in<int> Port_Addr;
	sc_out<RetCode> Port_Done;
	sc_inout_rv<32> Port_Data;

	SC_CTOR(Memory) {
		SC_THREAD(execute);
		sensitive << Port_CLK.pos();
		dont_initialize();
		m_data = new int[MEM_SIZE];
	}
	~Memory() {
		delete[] m_data;
	}

private:
	int * m_data;
	void execute() {
		while (true) {
			wait(Port_Func.value_changed_event());
			Function f = Port_Func.read();
			int addr = Port_Addr.read();
			int data = 0;
			if (f == FUNC_WRITE) {
				data = Port_Data.read().to_int();
			}
			// Simulate Memory read / write delay
			wait(100);
			if (f == FUNC_READ) {
				Port_Data.write((addr < MEM_SIZE) ? m_data[addr] : 0);
				Port_Done.write(RET_READ_DONE);
				wait();
				Port_Data.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
			} else {
				if (addr < MEM_SIZE) {
					m_data[addr] = data;
				}
				Port_Done.write(RET_WRITE_DONE);
			}
		}
	}
};

SC_MODULE(CPU) {
public:
	sc_in<bool> Port_CLK;
	sc_in<Memory::RetCode> Port_MemDone;
	sc_out<Memory::Function> Port_MemFunc;
	sc_out<int> Port_MemAddr;
	sc_inout_rv<32> Port_MemData;

	SC_CTOR(CPU) {
		SC_THREAD(execute);
		sensitive << Port_CLK.pos();
		dont_initialize();
	}
private:
	void execute() {
		while (true) {
			wait();
			Memory::Function f =
					(rand() % 10) < 5 ? Memory::FUNC_READ : Memory::FUNC_WRITE;
			int addr = (rand() % MEM_SIZE);

			Port_MemAddr.write(addr);
			Port_MemFunc.write(f);
			if (f == Memory::FUNC_WRITE) {
				Port_MemData.write(rand());
				wait();
				Port_MemData.write(
						"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
			}
			wait(Port_MemDone.value_changed_event());
			// Advance one cycle in simulated time
			wait();
		}
	}
};
#endif /* RAM_H_ */
