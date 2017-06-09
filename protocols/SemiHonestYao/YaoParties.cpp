/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
* Copyright (c) 2016 LIBSCAPI (http://crypto.biu.ac.il/SCAPI)
* This file is part of the SCAPI project.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
* We request that any publication and/or code referring to and/or based on SCAPI contain an appropriate citation to SCAPI, including a reference to
* http://crypto.biu.ac.il/SCAPI.
* 
* Libscapi uses several open source libraries. Please see these projects for any further licensing issues.
* For more information , See https://github.com/cryptobiu/libscapi/blob/master/LICENSE.MD
*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
*/


#include "YaoParties.hpp"
#include <tuple>
#ifndef _WIN32
#include "../../include/circuits/Compat.h"
#endif

vector<byte> readInputAsVector(string input_file) {
	auto sc = scannerpp::Scanner(new scannerpp::File(input_file));
	int inputsNumber = sc.nextInt();
	vector<byte> inputVector(inputsNumber);
	for (int i = 0; i < inputsNumber; i++)
		inputVector[i] = (byte)sc.nextInt();
	return inputVector;
}

/*********************************/
/*          PartyOne             */
/*********************************/

PartyOne::PartyOne(YaoConfig & yao_config) {
	//t = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
	this->yaoConfig = yao_config;

	SocketPartyData me(yao_config.sender_ip, 1213);
	SocketPartyData other(yao_config.receiver_ip, 1212);
	channel = make_shared<CommPartyTCPSynced>(io_service, me, other);

	// create the garbled circuit
	circuit = GarbledCircuitFactory::createCircuit(yao_config.circuit_file,
		GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);

	// create the semi honest OT extension sender
	SocketPartyData senderParty(yao_config.sender_ip, 7766);
#ifdef _WIN32
	otSender = new OTSemiHonestExtensionSender(senderParty, 163, 1);
#else
	otSender = new OTExtensionBristolSender(senderParty.getPort(), true, channel);
#endif

	// connect to party two
	channel->join(500, 5000);
	ungarbledInput = readInputAsVector(yao_config.input_file_1);
};

void PartyOne::setInputs(string inputFileName) {
	ungarbledInput = readInputAsVector(inputFileName);
}

void PartyOne::sendP1Inputs(byte* ungarbledInput) {
	byte* allInputs = (byte*)std::get<0>(values);
	// get the size of party one inputs
	int numberOfp1Inputs = 0;
	numberOfp1Inputs = circuit->getNumberOfInputs(1);
	int inputsSize = numberOfp1Inputs*SIZE_OF_BLOCK;
	byte* p1Inputs = new byte[inputsSize];

	// create an array with the keys corresponding the given input.
	int inputStartIndex;
	for (int i = 0; i < numberOfp1Inputs; i++) {
		inputStartIndex = (2 * i + ((int) ungarbledInput[i]))*SIZE_OF_BLOCK;
		memcpy(p1Inputs + i*SIZE_OF_BLOCK, allInputs + inputStartIndex, SIZE_OF_BLOCK);
	}
	// send the keys to p2.
	channel->write(p1Inputs, inputsSize);
	delete p1Inputs;
}

void PartyOne::run() {
	
	values = circuit->garble();

	// send garbled tables and the translation table to p2.
	auto garbledTables = circuit->getGarbledTables();
	
	channel->write((byte *) garbledTables, circuit->getGarbledTableSize());
	channel->write(circuit->getTranslationTable().data(), circuit->getNumberOfOutputs());
	
	// send p1 input keys to p2.
	sendP1Inputs(ungarbledInput.data());
	
	// run OT protocol in order to send p2 the necessary keys without revealing any information.
	runOTProtocol();
}

void PartyOne::runOTProtocol() {
	//Get the indices of p2 input wires.
	int p1InputSize = 0;
	int p2InputSize = 0;
	byte* allInputWireValues = (byte*)std::get<0>(values);
	p1InputSize = circuit->getNumberOfInputs(1);
	p2InputSize = circuit->getNumberOfInputs(2);
	vector<byte> x0Arr;
	x0Arr.reserve(p2InputSize * SIZE_OF_BLOCK);
	vector<byte> x1Arr;
	x1Arr.reserve(p2InputSize * SIZE_OF_BLOCK);
	int beginIndex0, beginIndex1;
	for (int i = 0; i<p2InputSize; i++) {
		beginIndex0 = p1InputSize * 2 * SIZE_OF_BLOCK + 2 * i*SIZE_OF_BLOCK;
		beginIndex1 = p1InputSize * 2 * SIZE_OF_BLOCK + (2 * i + 1)*SIZE_OF_BLOCK;
		x0Arr.insert(x0Arr.end(), &allInputWireValues[beginIndex0], &allInputWireValues[beginIndex0 + SIZE_OF_BLOCK]);
		x1Arr.insert(x1Arr.end(), &allInputWireValues[beginIndex1], &allInputWireValues[beginIndex1 + SIZE_OF_BLOCK]);
	}
	// create an OT input object with the keys arrays.
	OTBatchSInput * input = new OTExtensionGeneralSInput(x0Arr, x1Arr, p2InputSize);
	// run the OT's transfer phase.
	otSender->transfer(input);
}

/*********************************/
/*          PartyTwo             */
/*********************************/

PartyTwo::PartyTwo(YaoConfig & yao_config, bool print_output) {
	this->print_output = print_output;
	this->yaoConfig = yao_config;
	// init
	SocketPartyData me(yao_config.receiver_ip, 1212);
	SocketPartyData other(yao_config.sender_ip, 1213);
	channel = make_shared<CommPartyTCPSynced>(io_service, me, other);

	// create the garbled circuit
	circuit = GarbledCircuitFactory::createCircuit(yao_config.circuit_file,
		GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);

	// create the OT receiver.
	SocketPartyData senderParty(yao_config.sender_ip, 7766);
#ifdef _WIN32
	otReceiver = new OTSemiHonestExtensionReceiver(senderParty, 163, 1);
#else
	otReceiver = new OTExtensionBristolReceiver(senderParty.getIpAddress().to_string(), senderParty.getPort(), true, channel);
#endif


	// connect to party one
	channel->join(500, 5000);
	ungarbledInput = readInputAsVector(yao_config.input_file_2);
}

void PartyTwo::setInputs(string inputFileName) {
	ungarbledInput = readInputAsVector(inputFileName);
}

byte* PartyTwo::computeCircuit(OTBatchROutput * otOutput) {

	// Get the input of the protocol.
	vector<byte> p2Inputs = ((OTOnByteArrayROutput *)otOutput)->getXSigma();
	int p2InputsSize = ((OTOnByteArrayROutput *)otOutput)->getLength();
	// Get party two input wires' indices.
	vector<byte> allInputs(p1InputsSize + p2InputsSize);
	memcpy(&allInputs[0], p1Inputs, p1InputsSize);
	memcpy(&allInputs[p1InputsSize], p2Inputs.data(), p2InputsSize);
	
	// compute the circuit.
	block* garbledOutput = (block *)_aligned_malloc(sizeof(block) * 2 * circuit->getNumberOfOutputs(), SIZE_OF_BLOCK); ;
	circuit->compute((block *)&allInputs[0], garbledOutput);

	// translate the result from compute.
	byte* circuitOutput = new byte[circuit->getNumberOfOutputs()];
	circuit->translate(garbledOutput, circuitOutput);

	return circuitOutput;
}

void PartyTwo::run() {
	// receive tables and inputs
	receiveCircuit();
	receiveP1Inputs();

	// run OT protocol in order to get the necessary keys without revealing any information.
	auto output = runOTProtocol(ungarbledInput.data(), ungarbledInput.size());

	// Compute the circuit.
	auto outputBytes = computeCircuit(output.get());
	circuitOutput.resize(circuit->getNumberOfOutputs());
	memcpy(circuitOutput.data(), outputBytes, circuitOutput.size());

	// we're done print the output
	if (print_output)
	{
		int outputSize = circuit->getNumberOfOutputs();
		cout << "PartyTwo: printing outputSize: " << outputSize << endl;
		for (int i = 0; i < outputSize; i++)
			cout << (int)circuitOutput[i];
		cout << endl;
	}
}

void PartyTwo::receiveCircuit() {

	// receive garbled tables.
	channel->read((byte*)circuit->getGarbledTables(), circuit->getGarbledTableSize());
	
	byte * translationTable = new byte[circuit->getNumberOfOutputs()];

	// receive translation table.
	channel->read(translationTable, circuit->getNumberOfOutputs());

	std::vector<byte> translationTableVec(translationTable, translationTable + circuit->getNumberOfOutputs());
	
	// set garbled tables and translation table to the circuit.

	//TODO MEITAL remove after the comm layer changes
	//block* garbledTabledAligned = (block *)_aligned_malloc(circuit->getGarbledTableSize(), SIZE_OF_BLOCK);

	//copy to the aligned memory
	//memcpy(garbledTabledAligned, garbledTables, circuit->getGarbledTableSize());

	//circuit->setGarbledTables((block*)garbledTables);
	circuit->setTranslationTable(translationTableVec);
}

void PartyTwo::receiveP1Inputs() {
	p1InputsSize = circuit->getNumberOfInputs(1)*SIZE_OF_BLOCK;

	p1Inputs = new byte[p1InputsSize];

	channel->read(p1Inputs, p1InputsSize);
}
