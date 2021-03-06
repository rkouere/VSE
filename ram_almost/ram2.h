/*
 * ram2.h
 *
 *  Created on: Nov 10, 2015
 *      Author: rkouere
 */

#ifndef RAM2_H_
#define RAM2_H_
#include "systemc.h"

#define MEM_SIZE 512

SC_MODULE(Memory) {
public:
	enum Function_enum {
		FUNC_NONE, FUNC_READ, FUNC_WRITE
	};
	enum RETSignal {
		RSIG_NONE, RSIG_READ_FIN, RSIG_WRITE_FIN, RSIG_ERROR
	};

	sc_in<bool> Port_CLK;
	sc_in<Function_enum> Port_Func;
	sc_in<int> Port_Addr;
	sc_inout<int> Port_Data;
	sc_out<RETSignal> Port_DoneSig;

	SC_CTOR(Memory) {
		SC_METHOD(execute);
		sensitive << Port_CLK.neg();
		m_clkCnt = 0;
		m_curAddr = 0;
		m_curData = 0;
		m_curFunc = Memory::FUNC_NONE;
		m_data = new int[MEM_SIZE];
		m_writesCnt = 0;
		m_readsCnt = 0;
		m_errorsCnt = 0;
		m_errorCode = 0;

	}
	~Memory() {
		delete[] m_data;
	}
private:
	int Function;
	int m_clkCnt;
	int m_curAddr;
	int m_curData;
	int m_curFunc;

	int* m_data;
	int m_errorCode;
	int m_writesCnt;
	int m_readsCnt;
	int m_errorsCnt;

	RETSignal read() {
		if (m_errorCode == 0) {
			Port_Data.write(m_data[m_curAddr]);
			m_readsCnt++;
			return RSIG_READ_FIN;
		} else {
			m_errorsCnt++;
			return RSIG_ERROR;
		}
	}
	RETSignal write() {
		if (m_errorCode == 0) {
			m_data[m_curAddr] = m_curData;
			m_writesCnt++;
			return RSIG_WRITE_FIN;
		} else {
			m_errorsCnt++;
			return RSIG_ERROR;
		}
	}
	void execute() {
		if (m_curFunc != Memory::FUNC_NONE) {
			m_clkCnt++;
			if (m_clkCnt == 100) {
				RETSignal retSig = Memory::RSIG_ERROR;
				switch (m_curFunc) {
				case Memory::FUNC_READ: {
					retSig = read();
					break;
				}
				case Memory::FUNC_WRITE: {
					retSig = write();
					break;
				}
				}
				Port_DoneSig.write(retSig);
				m_clkCnt = 0;
				m_curFunc = Memory::FUNC_NONE;
			}
			return;
		}
		if (Port_Func.read() == Memory::FUNC_NONE) {
			return;
		}
		m_curFunc = Port_Func.read();

		m_curAddr = Port_Addr.read();
		m_curData = Port_Data.read();
		Port_DoneSig.write(Memory::RSIG_NONE);
	}
};

SC_MODULE(CPU) {
public:
	sc_in<bool> Port_CLK;
	sc_in<Memory::RETSignal> Port_MemDone;
	sc_out<Memory::Function_enum> Port_MemFunc;
	sc_out<int> Port_MemAddr;
	sc_inout<int> Port_MemData;

	SC_CTOR(CPU) {
		SC_METHOD(execCycle);
		sensitive << Port_CLK.pos();
		dont_initialize();
		SC_METHOD(memDone);
		sensitive << Port_MemDone;
		dont_initialize();
		m_waitMem = false;
	}
private:
	bool m_waitMem;
	Memory::Function_enum getrndfunc() {
		int rndnum = (rand() % 10);
		if (rndnum < 5)
			return Memory::FUNC_READ;
		else
			return Memory::FUNC_WRITE;
	}
	int getRndAddress() {
		return (rand() % MEM_SIZE);
	}
	int getRndData() {
		return rand();
	}
	void execCycle() {
		if (m_waitMem) {
			return;
		}
		int addr = getRndAddress();
		Memory::Function_enum f = getrndfunc();
		Port_MemFunc.write(f);
		Port_MemAddr.write(addr);
		if (f == Memory::FUNC_WRITE)
			Port_MemData.write(getRndData());
		m_waitMem = true;
	}
	void memDone() {
		if (Port_MemDone.read() == Memory::RSIG_NONE) {
			return;
		}
		m_waitMem = false;
		Port_MemFunc.write(Memory::FUNC_NONE);
	}
};
#endif /* RAM2_H_ */
