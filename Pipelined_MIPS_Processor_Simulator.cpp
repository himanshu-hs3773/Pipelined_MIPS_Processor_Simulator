#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 256 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public:
        bitset<32> Reg_data;
     	RF()
    	{
			Registers.resize(32);
			Registers[0] = bitset<32> (0);
        }

        bitset<32> readRF(bitset<5> Reg_addr)
        {
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }

		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();
		}

	private:
		vector<bitset<32> >Registers;
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{
					IMem[i] = bitset<8>(line);
					i++;
				}
			}
            else cout<<"Unable to open file";
			imem.close();
		}

		bitset<32> readInstr(bitset<32> ReadAddress)
		{
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;
		}

    private:
        vector<bitset<8> > IMem;
};

class DataMem
{
    public:
        bitset<32> ReadData;
        DataMem()
        {
            DMem.resize(MemSize);
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();
        }

        bitset<32> readDataMem(bitset<32> Address)
        {
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;
		}

        void writeDataMem(bitset<32> Address, bitset<32> WriteData)
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
        }

        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {
                    dmemout << DMem[j]<<endl;
                }

            }
            else cout<<"Unable to open file";
            dmemout.close();
        }

    private:
		vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}

unsigned long shiftbits(bitset<32> instr, int start)
{
    return ((instr.to_ulong())>>start);

}
bitset<32> signextend (bitset<16> imm)
{
    string sestring;
    if (imm[15]==0){
        sestring = "0000000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    else{
        sestring = "1111111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    return (bitset<32> (sestring));

}

bitset<32> branchingAddress (bitset<16> addr)
{
    string sestring;
    if (addr[15]==0){
        sestring = "00000000000000"+addr.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    else{
        sestring = "11111111111111"+addr.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    return (bitset<32> (sestring));

}

int main()
{
	// Initialization
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    bitset<6> opcode;
    bitset<6> funct;
    bitset<16> imm;

    //control signals
    /*
     *
    bool IType;
    bool RType;
    bool IsLoad;
    bool IsStore;
    bool WrtEnable;
	*/

    bool isBranch;
    bitset<32> bAddr;

    // define newstate

    stateStruct state, newState;
    int cycle = 0;

    //initializing states for first cycle

    state.WB.nop = 1;
    state.MEM.nop = 1;
    state.EX.nop = 1;
    state.ID.nop = 1;
    state.IF.nop = 0;




    state.IF.PC = 0;

    while (1) {

        /* --------------------- WB stage --------------------- */
    	if(!state.WB.nop && state.WB.wrt_enable)
    	{
    		// Write to the registers
    		myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
    	}

    	else if(state.WB.nop == 1){
    		// Do nothing

    	}



        /* --------------------- MEM stage --------------------- */
    	if(state.MEM.nop == 0){
    		// Write/Read to/from the memory
    		newState.WB.nop = 0;
    		newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;

			if(state.MEM.wrt_enable && !state.MEM.rd_mem && !state.MEM.wrt_mem)
			{
				newState.WB.Wrt_data = state.MEM.ALUresult;
			}

			else if(state.MEM.rd_mem)
			{
				newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);				//load word from memory
			}

			else if(state.MEM.wrt_mem)
			{
				myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
			}


    	}

			else if(state.MEM.nop == 1){
    		// Propagate nop to WB
				newState.WB.nop = 1;
			}


        /* --------------------- EX stage --------------------- */
    	if(state.EX.nop == 0){
    		// Perform operations
    		newState.MEM.nop = 0;

    		newState.MEM.Rs = state.EX.Rs;
    		newState.MEM.Rt = state.EX.Rt;
    		newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
    		newState.MEM.rd_mem = state.EX.rd_mem;
    		newState.MEM.wrt_enable = state.EX.wrt_enable;
    		newState.MEM.wrt_mem = state.EX.wrt_mem;


    		 if(!state.EX.alu_op)
    		 {

    			 if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
				 }
    			 else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
    		 	 {
    		  	  	  newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() - state.EX.Read_data2.to_ulong());
    		 	 }
    			 else if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
    		 	 {
    		 	 	 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.MEM.ALUresult.to_ulong());
    		 	 }
    			 else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() - state.MEM.ALUresult.to_ulong());
				 }

    		 	 else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
				 }
    		 	 else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
    		 	 {
    		 	 	 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.WB.Wrt_data.to_ulong());
    		 	 }
    		 	else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
				 {
					  newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() - state.EX.Read_data2.to_ulong());
				 }
    		 	else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() - state.WB.Wrt_data.to_ulong());
				 }

    		 }



    		else if(state.EX.alu_op && !state.EX.is_I_type)
    		 {
    			if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt  && state.MEM.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
				 }
				 else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt  && state.MEM.wrt_enable)
				 {
					  newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + state.EX.Read_data2.to_ulong());
				 }
				 else if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt  && state.MEM.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.MEM.ALUresult.to_ulong());
				 }
				 else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt  && state.MEM.wrt_enable)
				 {
					 newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + state.MEM.ALUresult.to_ulong());
				 }

				 else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt  && state.WB.wrt_enable)
    		 	 {
    		 	 	 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
    			 }

    		 	 else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt  && state.WB.wrt_enable)
    		 	 {
    		  	  	  newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + state.EX.Read_data2.to_ulong());
    		 	 }

    		 	 else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt  && state.WB.wrt_enable)
    		 	 {
    		 	 	 newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.WB.Wrt_data.to_ulong());
    		 	 }
    		 	 else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt  && state.WB.wrt_enable)
    		 	 {
    		 	 	 newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + state.WB.Wrt_data.to_ulong());
    		 	 }
    		 }


			 else if(state.EX.is_I_type && state.EX.rd_mem)
			 {

				 	if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}



					else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

					else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + signextend(state.EX.Imm).to_ulong());
					}

			 }
			else if(state.EX.is_I_type && state.EX.wrt_mem)
			 {

					if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2;
					}

					else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr != state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2;

					 }

					else if(state.MEM.Wrt_reg_addr != state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2; // new
					}

					else if(state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.MEM.ALUresult.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2; // new
					}



					else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2;
					}

					else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr != state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.EX.Read_data2;

					 }

					else if(state.WB.Wrt_reg_addr != state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.WB.Wrt_data;
					}

					else if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable)
					{
						newState.MEM.ALUresult = bitset<32> (state.WB.Wrt_data.to_ulong() + signextend(state.EX.Imm).to_ulong());
						newState.MEM.Store_data = state.WB.Wrt_data;
					}

			 }
    		 /*


    		//subu
    		if(!state.EX.alu_op)
    		{

    			newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
    			cout << newState.MEM.ALUresult << endl;
    		}

    		//addu
    		else if(state.EX.alu_op && !state.EX.is_I_type)
    		{
    			newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
    			cout << newState.MEM.ALUresult << endl;
    		}


    		//  M[R[rs]+SignExtImm] = R[rt]

    		else if(state.EX.is_I_type && !state.EX.rd_mem && state.EX.wrt_mem)
			{
				newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
				newState.MEM.Store_data = state.EX.Read_data2;
				cout << newState.MEM.ALUresult << endl;

			}
    		//load word R[rt] = M[R[rs]+SignExtImm]
    		else if(state.EX.is_I_type && state.EX.rd_mem && !state.EX.wrt_mem)
			{
				newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
				cout << newState.MEM.ALUresult << endl;

			}

    		cout << "-----------------------------------------" << endl;


*/

    	}

    	 else if(state.EX.nop == 1){
    		// Propagate nop to MEM
    		 newState.MEM.nop = 1;
    	}


        /* --------------------- ID stage --------------------- */
    	if(!state.ID.nop)
    	{
    		// Breaking the instruction
    		newState.EX.nop = 0;
    		opcode = bitset<6> (shiftbits(state.ID.Instr, 26));
    		funct = bitset<6> (shiftbits(state.ID.Instr, 0));
    		//isBranch = (opcode.to_ulong()==4)?1:0;

    		if (opcode.to_ulong()==4)
    		{
    			isBranch = true;
    		}
    		if(isBranch && (myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 21))) != myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 16))))){
    			newState.EX.nop = 1;
    			bAddr = branchingAddress(bitset<16> (shiftbits(state.ID.Instr, 0))).to_ulong() + state.IF.PC.to_ulong();
    		}

    		if(opcode.to_ulong() == 0){				//checking for itype
				newState.EX.is_I_type = false;				//when not itype - setting write enable to 1
				newState.EX.wrt_enable = true;
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = false;

			}
			else if(opcode.to_ulong() == 35) // load word 23hex
			{
				newState.EX.is_I_type = true;
				newState.EX.wrt_enable = true;
				newState.EX.rd_mem = true;
				newState.EX.wrt_mem = false;
			}
			else if(opcode.to_ulong() == 43) // store word 2bhex
			{
				newState.EX.is_I_type = true;
				newState.EX.wrt_enable = false;
				newState.EX.rd_mem = false;
				newState.EX.wrt_mem = true;
			}

    		newState.EX.Rs = bitset<5> (shiftbits(state.ID.Instr, 21));


    		newState.EX.Rt = bitset<5> (shiftbits(state.ID.Instr, 16));

    		if(newState.EX.is_I_type && newState.EX.wrt_enable)
    		{
    			newState.EX.Wrt_reg_addr = newState.EX.Rt;
    		}
    		else if(!newState.EX.is_I_type)
    		{
    			newState.EX.Wrt_reg_addr = bitset<5> (shiftbits(state.ID.Instr, 11));
    		}

    		newState.EX.Imm = bitset<16> (shiftbits(state.ID.Instr, 0));
    		newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
    		newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);







			// for alu_op

			if(opcode.to_ulong() == 0 && funct.to_ulong() == 35) // subu 23hex
			{
				newState.EX.alu_op = false;

			}
			else
			{
				newState.EX.alu_op = true;
			}

    		//end of ID.nop0 operations
    	}

    	else if(state.ID.nop){

    		newState.EX.nop = true;
    		// Propagate nop to EX
    	}




        /* --------------------- IF stage --------------------- */
    	if(!state.IF.nop){
    		// Fetch the instruction
    		if(isBranch && (myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 21))) != myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 16))))){
    			newState.ID.Instr = state.ID.Instr;
    			newState.IF.PC = bAddr;
    			newState.ID.nop = 1;
    		}else{
				newState.IF.nop = 0;
				newState.ID.nop = state.IF.nop; //sending nop0 - to next cycle
				newState.ID.Instr = myInsMem.readInstr(state.IF.PC); // value of progrm counter(instr) to ID/Rf

				if(newState.ID.Instr.to_string<char,std::string::traits_type,std::string::allocator_type>()=="11111111111111111111111111111111"){
				newState.ID.nop = 1;
				newState.IF.nop = 1;
				}

				else
				{
					newState.IF.PC = bitset<32> (state.IF.PC.to_ulong() + 4); //chng pc to decimal to add 4 and force concert to bitset for next state
				}
    		}
    	}

    	else if(state.IF.nop){
    		newState.ID.nop = 1;// Propagate nop to ID
    	}


        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...

        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
        cycle++;
    }

    myRF.outputRF(); // dump RF;
	myDataMem.outputDataMem(); // dump data mem

	return 0;
}
