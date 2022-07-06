//
// Created by moriya on 19/10/17.
//

#ifndef SCAPI_GARBLEDBOOLEANCIRCUITNOINTRINSICS_H
#define SCAPI_GARBLEDBOOLEANCIRCUITNOINTRINSICS_H

struct GarbledGate;
#define KEY_SIZE 16
#define XOR_GATE 6//the truth table is 0110
#define XOR_NOT_GATE 9// the truth table is 1001, can also use the optimization of FreeXor

#include "../infra/Common.hpp"
#include "GarbledGate.h"
#include <vector>
#include <tuple>
#include <openssl/rand.h>
#include "../primitives/PrfOpenSSL.hpp"

/*
 * GarbledBooleanCircuitNoIntrinsics is a general abstract class for garbled circuits.
 * All garbled circuits have four main operations:
 * 1. The garble function that creates the garbled table
 * 2. The compute function that computes a result on a garbled circuit on the input which is the keys that were chosen for each input wire.
 * 3. The verify method is used in the case of a malicious adversary to verify that the garbled circuit
 * created is an honest garbling of the agreed upon non garbled circuit.
 * 4. The translate method that translates the garbled output which usually is generated by the compute() function into meaningful output (a 0/1 result, rather
 * than the keys outputed by compute)
 *
 *	author: Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Meital Levy)
 *
 */
class GarbledBooleanCircuitNoIntrinsics
{

private:
    vector<uint8_t> deltaFreeXor;//This is used to get the second garbled value in freeXor optimization. The second garbled value is the XOR
    //of the first key and the delta. The delta is chosen at random.
    //We use a pointer since new with 32 bit does not 16-align the variable by default.


    vector<uint8_t> encryptedChunkKeys;//The result of chunk encrypting indexArray.
    vector<uint8_t> indexArray;//An array that holds the number 0 to the number of nonXorGates and is calculated in advence.
    //The purpuse of this array is that we can calculate. this array and all the keys of the circuit in advence using ecb mode
    //with one chuck gaining pipelining

    /*
    * This method generates both keys for each wire. Then, creates the garbled table according to those values.
    * It is a virtual function since each derived class generates the garbled table differently.
    * The keys for each wire are not saved. The input keys and the output keys that were created are returned to the
    * user. The user usually saves these value for later use. The user also gets the generated translation table, which is
    * the signal bits of the output wires.
    *
    * emptyBothInputKeys : An empty block array that will be filled with both input keys generated in garble.
    * emptyBothOutputKeys : An empty block array that will be filled with both output keys generated in garble.
    * emptyTranslationTable : An empty int array that will be filled with 0/1 signal bits that we chosen in random in this function.
    */
    void garble(uint8_t * emptyBothInputKeys, uint8_t * emptyBothOutputKeys, std::vector<uint8_t> & emptyTranslationTable, uint8_t* seed);

    /*
    * This function inits the keys for all the wires in the circuit and initializes the two aes encryptions (seed and fixedKey as keys). It also choses
    * the input keys at random using the aes with seed. It also creates memory for the translation table.
    */
    void initAesEncryptionsAndAllKeys(uint8_t* emptyBothInputKeys);

protected:

    bool isFreeXor;//A flag indicating if the user wants to use the optimization of FreeXor.

    bool isNonXorOutputsRequired;//A flag that indicates that the output wires randomized rather than have a common delta between the 0-wire and the 1-wire.

    int numberOfInputs;//The total number of inputs for all parties
    int numberOfOutputs;//Number of outputs
    int lastWireIndex;//The index of the last wire in the circuit. This determines the size of the wire array and thus should not be
    //much higher than the actual number of wires
    int numberOfGates;//The number of gates in the circuit (excluding the -1 index of the fixed 1 gate that provides a wire that allways
    //computes to the 1 garbled value.
    int numOfXorGates; //The number of XOR gates. This is used when the free xor optimization is used in order to use garbled table just
    //for the non-XOR gates.
    int numOfNotGates;//The number of NOT gates. This is used when the free xor optimization is used in order to use garbled table just
    //for the non-XOR gates.

    bool NOTgateOptimization;

    int numberOfParties;//The number parties participating in the protocol that uses the circuit.
    vector<int> numOfInputsForEachParty;//An array that holds for each party the number of inputs it has in the circuit.

    std::vector<int> inputIndices;//The indices of the input wires.
    std::vector<int> outputIndices;//The indices of the output wires.

    uint8_t* seed;//The seed is used to create the garbled value via AES. The seed is the key of the AES that generates random values.

    /*
      * We store the garbled tables in a one dimensional array of GarbleTable.
    * GarbleTable is an array of blocks(128 bit variable), the garbled in the array corresponds to the gate
      * Each table of each gate is a one dimensional array of bytes rather than an array of ciphertexts.
      * This is for time/space efficiency reasons: If a single garbled table is an array of ciphertext that holds a uint8_t array the space
      * stored by java is big. The main array holds references for each item (4 bytes). Each array in java has an overhead of 12 bytes.
      * Thus the garbled table with ciphertexts has at least (12+4)*number of rows overhead.
      * If we store the array as one dimensional array we only have 12 bytes overhead for the entire table and thus this is the way we
      * store the garbled tables.
      */
    uint8_t* garbledTables;


    GarbledGate *garbledGates;//An array that holds the garbled gates. This is fiiled by the derived classes according to the derived class needs
    /*
     * The translation table stores the signal bit for the output wires. Thus, it just tells you whether the wire coming out is a
     * 0 or 1 but does not reveal information about the 2 keys of the wire. This is good since it is possible that a circuit output
     * wire is also an input wire to a different gate, and thus if the translation table contained both keys
     * values of the output Wire, the constructing party could change the value of the wire when it is input into a gate, and
     * privacy and/or correctness will not be preserved. Therefore, we only reveal the signal bit, and the other
     * possible value for the wire is not stored on the translation table.
     */
    std::vector<uint8_t> translationTable;

    /*
    /two aes encryption schemes that will be used in all the derived classes
    /The seed is given by the user of the garbled circuit and the fixedKey is hardcoded
    */
//    SecretKey aesSeedKey;//We use a pointer since new with 32 bit does not 16-align the variable by default

    OpenSSLAES aes; //The object used to encrypt the keys
    //These values could have been defined locally in the functions that use them, however, if we define and allocate
    //these values in construction, the performance is better. Allocating the memory consumes negligible time, but using
    //values of these array without some initialization (in our case using memset) causes a lot of cache misses and thus
    //performance decrease. We make sure to delete these values in the destructor.
    uint8_t* garbledWires;
    uint8_t* computedWires;

    SecretKey aesFixedKey;//We use a pointer since new with 32 bit does not 16-align the variable by default

    int numOfRows;//number of rows

    /*
	* Creates the memory needed for this class in addition to the memory that is allocated by the base class.
	*/
    void createCircuitMemory(const char* fileName, bool isNonXorOutputsRequired);

    /**
    * turns the string of the truth table that was taken as a decimal number into a number between 0 and 15 which represents the truth table
    * 0 means the truth table of 0000 and 8 means 1000 and so on. The functions returns the decimal representation of the thruth table.
    */
    int binaryTodecimal(int n);

    /**
    * returns the signal bit if a block (128 bit). This bit the 8'th bit of the 128 bits
    */
    uint8_t getSignalBitOf(uint8_t* x) {return *x & 1;};


    //uint8_t getSignalBitOf(block x) { return _mm_extract_epi16(x,  0) % 2; };
    /**
    * This is a simple function that returns the results of 2^p where p=0,1,2,3. Pow of c++ is not used since it uses double casted to int
    * and this is time consuming. Since we only have 4 options and efficiency is important we use this naive function. Using shifts is also
    * a little bit slower.
    */
    int integerPow(int p);

    /*
    * This functions gets a truth tables represented as a decimal number between 0-15 and returns the result of row i,j.
    */
    int getRowTruthTableResult(int i, int j, uint8_t truthTable);

    /*
    * This function does the same as the above function getRowTruthTableResult. The difference is that it uses shifts.
    */
    int getRowTruthTableResultShifts(int rowNumber, uint8_t truthTable){ return (truthTable & (1 << (3 - rowNumber))) >> (3 - rowNumber); };

    /*
    * In some cases it is important that the output keys of each wire will not all have the same fixed delta xor
    * between the 0-wire and the 1-wire (for example when the (constructor of the circuit) garbler needs to send both outputs
    * to the party that computes the circuit, if the delta is the same it breaks the security since the delta of the
    * the entire circuit is revealed.
    *
    * This function garbles the output keys using identity gates, the computing party will need to use the version
    * that decrypts these identity gates by turning the flag isNonXorOutputsRequired to true.
    * Garble calls this function only when this flag is true.
    */
//    void garbleOutputWiresToNoFixedDelta(uint8_t *deltaFreeXor, int nonXorIndex, uint8_t *emptyBothOutputKeys);

    /*
    * In some cases it is important that the output keys of each wire will not all have the same fixed delta xor
    * between the 0-wire and the 1-wire (for example when the (constructor of the circuit) garbler needs to send the both outputs
    * to the party that computes the circuit, if the delta is the same it breaks the security since the delta of the
    * the entire circuit is revealed.
    *
    * This function computes the identity gates that the garbeler had created.
    */
//    void computeOutputWiresToNoFixedDelta(int nonXorIndex, uint8_t * Output);

    /*
    * In some cases it is important that the output keys of each wire will not all have the same fixed delta xor
    * between the 0-wire and the 1-wire (for example when the (constructor of the circuit) garbler needs to send the both outputs
    * to the party that computes the circuit, if the delta is the same it breaks the security since the delta of the
    * the entire circuit is revealed.
    *
    * This function just turns the keys with fixed delta between them to the keys without delta using the seed in the same way that
    * the garble function does. It does not create a garbled table for thes identity gates since there is no use.
    * The last verification will be done in verifyTranslationTable that makes sure that the new outputs with no fixed delta
    * have signal bits matching the corresponding bit in the translation table.
    */
//    void verifyOutputWiresToNoFixedDelta(uint8_t *bothOutputsKeys);

public:
    GarbledBooleanCircuitNoIntrinsics(const char* fileName, bool isNonXorOutputsRequired=false);

    virtual ~GarbledBooleanCircuitNoIntrinsics(void);

    /*
	* This method generates calls the undelying implementation of garble in the derived classes using the seed.
	* It generates inputs outputs and tranlation table and passes that to the underlying garble to fill.
	* The keys for each wire are not saved. The input keys and the output keys that were created are returned to the
	* user. The user usually saves these value for later use. The user also gets the generated translation table, which is
	* the signal bits of the output wires.
	*
	* return values in the tuple:
	* block * : A block array that will be filled with both input keys generated in garble.
	* block * : An empty block array that will be filled with both output keys generated in garble.
	* uint8_t : An char array that will be filled with 0/1 signal bits that we chosen in random in this function.
	*/
    std::tuple<uint8_t*, uint8_t*, std::vector<uint8_t> > garble(uint8_t *seed = nullptr);


    /**
     * This method computes the circuit for the given input singleWiresInputKeys.
     * It returns a the garbled values keys for the output wires. This output can be translated via the translate() method
     * if the translation table is set.
     */
    void compute(uint8_t *singleWiresInputKeys,uint8_t * Output);

    /**
     * The verify method is used in the case of malicious adversaries.
     * For example, Alice can constructs n circuits and Bob can verify n-1 of them (of his choice) to confirm that they are indeed garbling of the
     * agreed upon non garbled circuit. In order to verify, Alice has to give Bob both keys for each of the input wires.

     * bothInputKeys: An array containing both keys for each input wire. For each input wire ,
     * the first garbled value is the 0 encoding, that is the 0 garbled value, and the second value is the 1 encoding.
     * returns:  true, if this GarbledBooleanCircuitNoIntrinsics is a garbling the given keys, false otherwise.
     */
    bool verify(uint8_t *bothInputKeys);

    /**
     * This function does the last part of the verify function. It gets both keys of each output wire and checks that
     * their signal bits match the corresponding bit in the translation table.
     *
     * The circuit verify is composed of both internalVerify function followed by this function.
     * @param allOutputWireValues both keys of each output wire.
     * @return {@code true} if the given keys match the translation table ,{@code false} if not.
     */
    bool verifyTranslationTable(uint8_t * emptyBothWireOutputKeys);

    /**
     * This is a virtual function that is implemented in the derived classes due to many changes in each derived class.
     * This function behaves exactly as the verify method except the last phase.
     * The verify function verifies that the translation table matches the resulted output garbled values, while this function does not, rather,
     * it returns the resulted output garbled values.
     *
     * bothWiresInputKeys : both keys for each input wire. This array must be filled with both input keys
     * emptyBothWireOutputKeys :T his array will be filled with both output keys during the process of the function. It must be empty.
     *
     * returns : true if this GarbledBooleanCircuitNoIntrinsics is a garbling the given keys, false otherwise.
     */
    bool internalVerify(uint8_t *bothWiresInputKeys, uint8_t *emptyBothWireOutputKeys);


    /**
    * Reads a file and fill the gates, number of parties, input indices, output indices and so on.
    * In addition initializes the member variables such as isFreeXor, isRowReduction and other variables that are set to nullptr.
    * It uses a fixed pre-defined fixed key and initializes the openSsl object accordingly.
    * It also allocates memory according to the circuit it creates, such as computedWires, garbledWires and so on. This
    * is done in the construction, rather than in the relevant function to reduce the cache misses.
    */
    virtual void createCircuit(const char* fileName, bool isFreeXor, bool isNonXorOutputsRequired=false);

    /**
    * This method read text file and creates an object of GarbledBooleanCircuitNoIntrinsics according to the file.
    * This includes creating the gates and other information about the parties involved. It also counts the number of XORGATES
    * in order to allocate garbled tables only for the non-XOR gates.
    *
    */
    void readCircuitFromFile(const char* fileName);


    /**
     * Translates the garbled output usually obtained from the compute function into a meaningful(i.e. 0-1) output.
     *
     * outputKeys : An array that contains a single garbled value for each output wire.
     * return: An array of chars with values 0/1
     */
    void translate(uint8_t *outputKeys, uint8_t* answer);


    /**
     * Returns the translation table of the circuit.
     * This is necessary since the constructor of the circuit may want to pass the translation table to an other party.
     * Usually, this will be used when the other party (not the constructor of the circuit) creates a circuit, sets the garbled tables
     * and needs the translation table as well to complete the computation of the circuit.
     * returns : the translation table of the circuit..
     */
    std::vector<uint8_t> getTranslationTable() { return translationTable; };

    /**
     * Sets the translation table of the circuit.
     * This is necessary when the garbled tables were set and we would like to compute the circuit later on.
     * translationTable : This value should match the garbled tables of the circuit.
     */
    void setTranslationTable(std::vector<uint8_t> & translationTable);

    /**
     * The garbled tables are stored in the circuit for all the gates. This method returns the garbled tables which is a GarbledTable array
     * equivalent to the number of the garbled gates.
     * This function is useful if we would like to pass garbled circuits built on different machines.
     * This is a compact way to define a circuit, that is, two garbled circuit with the same
     * basic boolean circuit structure only differ in the garbled tables and the translation table.
     * Thus we can hold one circuit and only replace the garbled tables and translation table and get a new garbled circuit.
     * The advantage is that the size of the tables only, is much smaller than all the information stored in the circuit .
     * (gates and other member variables). The size becomes important when sending large circuits.
     *
     * get/setGarbledTable() are used to retrieve/set the garbled tables.
     *
     */
    uint8_t* getGarbledTables() { return garbledTables;};
    void setGarbledTables(uint8_t* garbledTables);

    /**
    * Checks if two blocks have the same value.
    */
    bool equalBlocks(uint8_t* a, uint8_t* b);


    /**
    * Get function for class members that may be needed by users of the class, rather than a derived class.
    */
    vector<int> getNumOfInputsForEachParty();
    std::vector<int> getOutputIndices() { return outputIndices;};
    int getNumberOfOutputs() {return numberOfOutputs;};
    int getNumberOfInputs() { return numberOfInputs;};
    int getNumberOfInputs(int partyNumber) { return numOfInputsForEachParty[partyNumber - 1]; };
    std::vector<int> getInputIndices() {return inputIndices;};
    int getNumberOfParties() { return numberOfParties;};
    int getNumberOfGates() { return numberOfGates;};
    int getNumOfXorGates() { return numOfXorGates;};
    int getNumOfNotGates() { return numOfNotGates; };
    int getLastWireIndex() const { return lastWireIndex; };
    bool getIsFreeXor() const { return isFreeXor; };
    int getGarbledTableSize();
    std::vector<int> getInputWireIndices(int partyNumber);

    bool getIsNonXorOutputsRequired() const{ return isNonXorOutputsRequired; };

    int getNumOfRows() const { return numOfRows; };





};

#endif //SCAPI_GARBLEDBOOLEANCIRCUITNOINTRINSICS_H
