#include <iostream>
#include <fstream>
#include <streambuf>
#include <thread>
#include <mutex>

#include "DCSolver.h"
#include "TransientSolver.h"
#include "Net.h"
#include "Component.h"
#include "PassiveComponents.h"
#include "DiscreteSemis.h"
#include "Circuit.h"

Circuit circuit;

std::vector<std::string> lineBuffer;
std::mutex lineBufferMutex;

void interactiveTick(TransientSolver *solver) {
	std::cout << "RESULT " << solver->GetTimeAtTick(solver->GetCurrentTick()) << ",";
	for (int i = 0; i < circuit.Nets.size(); i++) {
		std::cout << solver->GetNetVoltage(circuit.Nets[i]) << ",";
	}
	for (int i = 0; i < circuit.Components.size(); i++) {
		for (int j = 0; j < circuit.Components[i]->GetNumberOfPins(); j++) {
			std::cout << solver->GetPinCurrent(circuit.Components[i], j) << ",";
		}
	}
	std::cout << std::endl;
	lineBufferMutex.lock();
	for each(std::string line in lineBuffer) {
		std::stringstream ss(line);
		
		std::string part;
		std::vector<std::string> parts;
		while (std::getline(ss, part, ' ')) {
			parts.push_back(part);	
		}
		if (parts.size() > 2) {
			if (parts[0] == "CHANGE") {
				for each(Component *c in circuit.Components) {
					if (c->ComponentID == parts[1]) {
						c->SetParameters(ParameterSet(parts));
					}
				}
			}
		}
	}
	lineBuffer.clear();
	lineBufferMutex.unlock();
}



void iothread() {
	std::string line;
	while (true) {
		std::getline(std::cin, line);
		if (line == "CONTINUE") {
			circuit.ContinueFromError = true;
		}
		else {
			lineBufferMutex.lock();
			lineBuffer.push_back(line);
			lineBufferMutex.unlock();
		}

	}
}

int main(int argc, char* argv[])
{
	std::string line = "";
	std::string netlist = "";
	char buf[2048];
	double simSpeed = 0;
	while (1) {
		std::cin.getline(buf, 2048);
		line = std::string(buf);
		if (line.find("START") != std::string::npos) {
			std::string s = line.substr(6);
			auto c = s.find(",");
			if (c != std::string::npos) {
				simSpeed = atof(s.replace(c, 1, ".").c_str());
			}
			else
			{
				simSpeed = atof(s.c_str());
			}
			break;
		}
		netlist.append(line);
		netlist.append("\n");
	}
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\nRES IC1.R1 _power_V+ _signal_0.A13  res=5000\r\nRES IC1.R2 _signal_0.A13 IC1.VL res=5000\r\nRES IC1.R3 IC1.VL _power_GND res=5000\r\nOPAMP IC1.U1 _signal_0.A12 _signal_0.A13 IC1.UC _power_GND _power_V+\r\nOPAMP IC1.U2 IC1.VL _signal_0.B11 IC1.LC _power_GND _power_V+\r\nLOGIC_NOT IC1.U3 _power_V+ IC1.R_INV _power_GND _power_V+\r\nLOGIC_OR IC1.U4 IC1.R_INV IC1.UC IC1.RES _power_GND _power_V+\r\nLOGIC_RS_FLIP_FLOP IC1.U5 IC1.RES IC1.LC _signal_0.B12 IC1.QB _power_GND _power_V+\r\nRES IC1.R4 IC1.QB IC1.DC res=1000\r\nBJT IC1.Q1 _signal_0.A12 IC1.DC _power_GND type=npn is=19e-15 bf=150 br=7.5 rb=50 re=0.4 rc=0.3\r\nRES R1 _signal_0.A12 _power_V+ res=100000\r\nCAP C1 _signal_0.A12 _power_GND rser=0.001 cap=0,0001\r\nRES SW1.1 _power_GND _signal_0.B11 res=1000000000000\r\nRES SW1.2 _power_GND _power_GND res=0,001\r\nRES SW1.3 _signal_0.B11 _signal_0.B11 res=0,001\r\nRES R2 _power_V+ _signal_0.B11 res=10000\r\nRES R3 _signal_0.B12 _signal_0.B19 res=330\r\nDIODE D1 _signal_0.B19 _power_GND is=1e-12 n=3 rser=9\r\n\r\n\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\nRES IC1.R1 _power_V+ _signal_0.A13  res=5000\r\nRES IC1.R2 _signal_0.A13 IC1.VL res=5000\r\nRES IC1.R3 IC1.VL _power_GND res=5000\r\nOPAMP IC1.U1 _signal_0.B4 _signal_0.A13 IC1.UC _power_GND _power_V+\r\nOPAMP IC1.U2 IC1.VL _signal_0.B4 IC1.LC _power_GND _power_V+\r\nLOGIC_NOT IC1.U3 _power_V+ IC1.R_INV _power_GND _power_V+\r\nLOGIC_OR IC1.U4 IC1.R_INV IC1.UC IC1.RES _power_GND _power_V+\r\nLOGIC_RS_FLIP_FLOP IC1.U5 IC1.RES IC1.LC _signal_0.B12 IC1.QB _power_GND _power_V+\r\nRES IC1.R4 IC1.QB IC1.DC res=1000\r\nBJT IC1.Q1 _signal_0.A3 IC1.DC _power_GND type=npn is=19e-15 bf=150 br=7.5 rb=50 re=0.4 rc=0.3\r\nRES R1 _signal_0.B12 _signal_0.B17 res=330\r\nDIODE D1 _signal_0.B17 _power_GND is=1e-12 n=3 rser=9\r\nRES R2 _signal_0.A3 _power_V+ res=100000\r\nCAP C1 _signal_0.B4 _power_GND rser=0.001 cap=1E-05\r\nRES VR1.1 _signal_0.B2 _signal_0.A3 res=62025,3164556963\r\nRES VR1.2 _signal_0.A3 _signal_0.B4 res=37974,6835443037\r\n\r\n\r\n\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\nLOGIC_NOR IC1a _signal_0.B1 _signal_0.B11 _signal_0.B12 _power_GND _power_V+\r\nLOGIC_NOR IC1b _signal_0.B15 _signal_0.B14 _signal_0.B13 _power_GND _power_V+\r\nLOGIC_NOR IC1c _signal_0.A16 _signal_0.A15 _signal_0.A14 _power_GND _power_V+\r\nLOGIC_NOR IC1d _signal_0.A11 _signal_0.A12 _signal_0.A13 _power_GND _power_V+\r\nRES SW1.1 _power_V+ _signal_0.B1 res=1000000000000\r\nRES SW1.2 _signal_0.B1 _power_GND res=0,001\r\nRES SW2.1 _power_V+ _signal_0.B11 res=1000000000000\r\nRES SW2.2 _signal_0.B11 _power_GND res=0,001\r\nRES R1 _signal_0.B12 _signal_0.B21 res=330\r\nDIODE D1 _signal_0.B21 _power_GND is=1e-12 n=3 rser=9\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\nLOGIC_NOR IC1a _signal_0.B1 _signal_0.B11 _signal_0.B12 _power_GND _power_V+\r\nLOGIC_NOR IC1b _signal_0.B15 _signal_0.B14 _signal_0.B13 _power_GND _power_V+\r\nLOGIC_NOR IC1c _signal_0.A16 _signal_0.A15 _signal_0.A14 _power_GND _power_V+\r\nLOGIC_NOR IC1d _signal_0.A11 _signal_0.A12 _signal_0.A13 _power_GND _power_V+\r\nRES SW1.1 _power_V+ _signal_0.B1 res=1000000000000\r\nRES SW1.2 _signal_0.B1 _power_GND res=0,001\r\nRES SW2.1 _power_V+ _signal_0.B11 res=1000000000000\r\nRES SW2.2 _signal_0.B11 _power_GND res=0,001\r\nRES R1 _signal_0.B12 _signal_0.B21 res=330\r\nDIODE D1 _signal_0.B21 _power_GND is=1e-12 n=3 rser=9\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\n\r\nAND MEM1a0 _power_V+ _signal_0.A22 MEM1.AO0 _power_GND _power_V+\r\nAND MEM1a1 _power_V+ _signal_0.A22 MEM1.AO1 _power_GND _power_V+\r\nAND MEM1a2 _signal_0.B21 _signal_0.A22 MEM1.AO2 _power_GND _power_V+\r\nAND MEM1a3 _power_V+ _signal_0.A22 MEM1.AO3 _power_GND _power_V+\r\nAND MEM1a4 _power_V+ _signal_0.A22 MEM1.AO4 _power_GND _power_V+\r\nAND MEM1a5 _signal_0.B18 _signal_0.A22 MEM1.AO5 _power_GND _power_V+\r\nAND MEM1a6 _signal_0.B17 _signal_0.A22 MEM1.AO6 _power_GND _power_V+\r\nAND MEM1a7 _signal_0.B16 _signal_0.A22 MEM1.AO7 _power_GND _power_V+\r\nAND MEM1a8 _signal_0.A17 _signal_0.A22 MEM1.AO8 _power_GND _power_V+\r\nAND MEM1a9 _signal_0.A18 _signal_0.A22 MEM1.AO9 _power_GND _power_V+\r\nAND MEM1a10 _signal_0.A21 _signal_0.A22 MEM1.AO10 _power_GND _power_V+\r\nAND MEM1a11 _signal_0.A19 _signal_0.A22 MEM1.AO11 _power_GND _power_V+\r\nAND MEM1a12 _signal_0.B15 _signal_0.A22 MEM1.AO12 _power_GND _power_V+\r\nAND MEM1a13 _signal_0.A16 _signal_0.A22 MEM1.AO13 _power_GND _power_V+\r\nAND MEM1a14 _signal_0.A15 _signal_0.A22 MEM1.AO14 _power_GND _power_V+\r\nAND MEM1a15 _signal_0.B14 _signal_0.A22 MEM1.AO15 _power_GND _power_V+\r\n\r\nAND MEM1d0 _signal_0.B24 _signal_0.A20 MEM1.DO0 _power_GND _power_V+\r\nAND MEM1d1 _signal_0.B25 _signal_0.A20 MEM1.DO1 _power_GND _power_V+\r\nAND MEM1d2 _signal_0.B26 _signal_0.A20 MEM1.DO2 _power_GND _power_V+\r\nAND MEM1d3 _signal_0.A27 _signal_0.A20 MEM1.DO3 _power_GND _power_V+\r\nAND MEM1d4 _signal_0.A26 _signal_0.A20 MEM1.DO4 _power_GND _power_V+\r\nAND MEM1d5 _signal_0.A25 _signal_0.A20 MEM1.DO5 _power_GND _power_V+\r\nAND MEM1d6 _signal_0.A24 _signal_0.A20 MEM1.DO6 _power_GND _power_V+\r\nAND MEM1d7 _signal_0.A23 _signal_0.A20 MEM1.DO7 _power_GND _power_V+\r\n\r\nROM MEM1b MEM1.AO15 MEM1.AO12 MEM1.AO7 MEM1.AO6 MEM1.AO5 MEM1.AO4 MEM1.AO3 MEM1.AO2 MEM1.AO1 MEM1.AO0 MEM1.DO0 MEM1.DO1 MEM1.DO2 _power_GND.VSS MEM1.DO3 MEM1.DO4 MEM1.DO5 MEM1.DO6 MEM1.DO7 MEM1.AO10 MEM1.AO11 MEM1.AO9 MEM1.AO8 MEM1.AO13 MEM1.AO14 _power_V+.VCC ires=1000000 ores=0.001\r\n\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\n\r\nLOGIC_AND MEM1a0 _power_V+ _power_V+ MEM1.AO0 _power_GND _power_V+\r\nLOGIC_AND MEM1a1 _power_V+ _power_V+ MEM1.AO1 _power_GND _power_V+\r\nLOGIC_AND MEM1a2 _signal_0.B21 _power_V+ MEM1.AO2 _power_GND _power_V+\r\nLOGIC_AND MEM1a3 _power_V+ _power_V+ MEM1.AO3 _power_GND _power_V+\r\nLOGIC_AND MEM1a4 _power_V+ _power_V+ MEM1.AO4 _power_GND _power_V+\r\nLOGIC_AND MEM1a5 _signal_0.B18 _power_V+ MEM1.AO5 _power_GND _power_V+\r\nLOGIC_AND MEM1a6 _signal_0.B17 _power_V+ MEM1.AO6 _power_GND _power_V+\r\nLOGIC_AND MEM1a7 _signal_0.B16 _power_V+ MEM1.AO7 _power_GND _power_V+\r\nLOGIC_AND MEM1a8 _signal_0.A17 _power_V+ MEM1.AO8 _power_GND _power_V+\r\nLOGIC_AND MEM1a9 _signal_0.A18 _power_V+ MEM1.AO9 _power_GND _power_V+\r\nLOGIC_AND MEM1a10 _signal_0.A21 _power_V+ MEM1.AO10 _power_GND _power_V+\r\nLOGIC_AND MEM1a11 _signal_0.A19 _power_V+ MEM1.AO11 _power_GND _power_V+\r\nLOGIC_AND MEM1a12 _signal_0.B15 _power_V+ MEM1.AO12 _power_GND _power_V+\r\nLOGIC_AND MEM1a13 _signal_0.A16 _power_V+ MEM1.AO13 _power_GND _power_V+\r\nLOGIC_AND MEM1a14 _signal_0.A15 _power_V+ MEM1.AO14 _power_GND _power_V+\r\nLOGIC_AND MEM1a15 _signal_0.B14 _power_V+ MEM1.AO15 _power_GND _power_V+\r\n\r\n\r\nMEM MEM1b MEM1.AO0 MEM1.AO1 MEM1.AO2 MEM1.AO3 MEM1.AO4 MEM1.AO5 MEM1.AO6 MEM1.AO7 MEM1.AO8 MEM1.AO9 MEM1.AO10 MEM1.AO11 MEM1.AO12 MEM1.AO13 MEM1.AO14 MEM1.AO15 MEM1.DO0 MEM1.DO1 MEM1.DO2 MEM1.DO3 MEM1.DO4 MEM1.DO5 MEM1.DO6 MEM1.DO7 _power_GND.VSS _power_V+.VCC ires=1000000 ores=0.001\r\n\r\nLOGIC_AND MEM1d0 MEM1.D0 _power_V+ MEM1.DD0 _power_GND _power_V+\r\nLOGIC_AND MEM1d1 MEM1.D1 _power_V+ MEM1.DD1 _power_GND _power_V+\r\nLOGIC_AND MEM1d2 MEM1.D2 _power_V+ MEM1.DD2 _power_GND _power_V+\r\nLOGIC_AND MEM1d3 MEM1.D3 _power_V+ MEM1.DD3 _power_GND _power_V+\r\nLOGIC_AND MEM1d4 MEM1.D4 _power_V+ MEM1.DD4 _power_GND _power_V+\r\nLOGIC_AND MEM1d5 MEM1.D5 _power_V+ MEM1.DD5 _power_GND _power_V+\r\nLOGIC_AND MEM1d6 MEM1.D6 _power_V+ MEM1.DD6 _power_GND _power_V+\r\nLOGIC_AND MEM1d7 MEM1.D7 _power_V+ MEM1.DD7 _power_GND _power_V+\r\n\r\nLOGIC_AND MEM1d0 MEM1.DD0 _power_V+ _signal_0.B24 _power_GND _power_V+\r\nLOGIC_AND MEM1d1 MEM1.DD1 _power_V+ _signal_0.B25 _power_GND _power_V+\r\nLOGIC_AND MEM1d2 MEM1.DD2 _power_V+ _signal_0.B26 _power_GND _power_V+\r\nLOGIC_AND MEM1d3 MEM1.DD3 _power_V+ _signal_0.A27 _power_GND _power_V+\r\nLOGIC_AND MEM1d4 MEM1.DD4 _power_V+ _signal_0.A26 _power_GND _power_V+\r\nLOGIC_AND MEM1d5 MEM1.DD5 _power_V+ _signal_0.A25 _power_GND _power_V+\r\nLOGIC_AND MEM1d6 MEM1.DD6 _power_V+ _signal_0.A24 _power_GND _power_V+\r\nLOGIC_AND MEM1d7 MEM1.DD7 _power_V+ _signal_0.A23 _power_GND _power_V+\r\n\r\n";
	//netlist = "NET _power_V+ 5\r\nNET _power_GND 0\r\nNET _power_V- -5\r\nLOGIC_AND MEM1a0 _power_V+ _power_V+ MEM1.A0 _power_GND _power_V+\r\nLOGIC_AND MEM1a1 _power_V+ _power_V+ MEM1.A1 _power_GND _power_V+\r\nLOGIC_AND MEM1a2 _signal_0.B21 _power_V+ MEM1.A2 _power_GND _power_V+\r\nLOGIC_AND MEM1a3 _power_V+ _power_V+ MEM1.A3 _power_GND _power_V+\r\nLOGIC_AND MEM1a4 _power_V+ _power_V+ MEM1.A4 _power_GND _power_V+\r\nLOGIC_AND MEM1a5 _signal_0.B18 _power_V+ MEM1.A5 _power_GND _power_V+\r\nLOGIC_AND MEM1a6 _signal_0.B17 _power_V+ MEM1.A6 _power_GND _power_V+\r\nLOGIC_AND MEM1a7 _signal_0.B16 _power_V+ MEM1.A7 _power_GND _power_V+\r\nLOGIC_AND MEM1a8 _signal_0.A17 _power_V+ MEM1.A8 _power_GND _power_V+\r\nLOGIC_AND MEM1a9 _signal_0.A18 _power_V+ MEM1.A9 _power_GND _power_V+\r\nLOGIC_AND MEM1a10 _signal_0.A21 _power_V+ MEM1.A10 _power_GND _power_V+\r\nLOGIC_AND MEM1a11 _signal_0.A19 _power_V+ MEM1.A11 _power_GND _power_V+\r\nLOGIC_AND MEM1a12 _signal_0.B15 _power_V+ MEM1.A12 _power_GND _power_V+\r\nLOGIC_AND MEM1a13 _signal_0.A16 _power_V+ MEM1.A13 _power_GND _power_V+\r\nLOGIC_AND MEM1a14 _signal_0.A15 _power_V+ MEM1.A14 _power_GND _power_V+\r\nLOGIC_AND MEM1a15 _signal_0.B14 _power_V+ MEM1.A15 _power_GND _power_V+\r\nRES MEM1.RA0 MEM1.A0 _power_GND res=1000\r\nRES MEM1.RA1 MEM1.A1 _power_GND res=1000\r\nRES MEM1.RA2 MEM1.A2 _power_GND res=1000\r\nRES MEM1.RA3 MEM1.A3 _power_GND res=1000\r\nRES MEM1.RA4 MEM1.A4 _power_GND res=1000\r\nRES MEM1.RA5 MEM1.A5 _power_GND res=1000\r\nRES MEM1.RA6 MEM1.A6 _power_GND res=1000\r\nRES MEM1.RA7 MEM1.A7 _power_GND res=1000\r\nRES MEM1.RA8 MEM1.A8 _power_GND res=1000\r\nRES MEM1.RA9 MEM1.A9 _power_GND res=1000\r\nRES MEM1.RA10 MEM1.A10 _power_GND res=1000\r\nRES MEM1.RA11 MEM1.A11 _power_GND res=1000\r\nRES MEM1.RA12 MEM1.A12 _power_GND res=1000\r\nRES MEM1.RA13 MEM1.A13 _power_GND res=1000\r\nRES MEM1.RA14 MEM1.A14 _power_GND res=1000\r\nRES MEM1.RA15 MEM1.A15 _power_GND res=1000\r\nLOGIC_AND MEM1di _power_V+ _power_V+ MEM1.DI _power_GND _power_V+\r\nLOGIC_AND MEM1d0 _power_V+ MEM1.D0 _power_GND _power_V+\r\nLOGIC_AND MEM1d1 _power_V+ MEM1.D1 _power_GND _power_V+\r\nLOGIC_AND MEM1d2 _power_V+ MEM1.D2 _power_GND _power_V+\r\nLOGIC_AND MEM1d3 _power_V+ MEM1.D3 _power_GND _power_V+\r\nLOGIC_AND MEM1d4 _power_V+ MEM1.D4 _power_GND _power_V+\r\nLOGIC_AND MEM1d5 _power_V+ MEM1.D5 _power_GND _power_V+\r\nLOGIC_AND MEM1d6 _power_V+ MEM1.D6 _power_GND _power_V+\r\nLOGIC_AND MEM1d7 _power_V+ MEM1.D7 _power_GND _power_V+\r\nRES MEM1.RD0 MEM1.D0 _signal_0.B24 res=0,001\r\nRES MEM1.RD1 MEM1.D1 _signal_0.B25 res=0,001\r\nRES MEM1.RD2 MEM1.D2 _signal_0.B26 res=0,001\r\nRES MEM1.RD3 MEM1.D3 _power_GND res=0,001\r\nRES MEM1.RD4 MEM1.D4 _signal_0.A27 res=1000000000000\r\nRES MEM1.RD5 MEM1.D5 _signal_0.A26 res=1000000000000\r\nRES MEM1.RD6 MEM1.D6 _signal_0.A25 res=1000000000000\r\nRES MEM1.RD7 MEM1.D7 _signal_0.A24 res=1000000000000\r\n\r\n";
	circuit.ReadNetlist(netlist);
	std::cout << "VARS t,";

	for (int i = 0; i < circuit.Nets.size(); i++) {
		std::cout << "V(" << circuit.Nets[i]->NetName << "),";
	}
	for (int i = 0; i < circuit.Components.size(); i++) {
		for (int j = 0; j < circuit.Components[i]->GetNumberOfPins(); j++) {
			std::cout << "I(" << circuit.Components[i]->ComponentID << "." << j << "),";
		}
	}
	std::cout << std::endl;

	DCSolver solver(&circuit);
	bool result = false;
	try {
		result = solver.Solve();
	}
	catch (void *e){
		std::cerr << "Failed to obtain initial operating point" << std::endl;
		circuit.ReportError("EXCEPTION", true);
	}
	if (!result) {
		circuit.ReportError("CONVERGENCE", false);
	}


	TransientSolver tranSolver(solver);
	tranSolver.InteractiveCallback = interactiveTick;
	std::thread updaterThread(iothread);
	tranSolver.RunInteractive(simSpeed);
	return 0;
}

