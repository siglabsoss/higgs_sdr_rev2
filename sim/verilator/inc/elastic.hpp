#include <stdlib.h>
#include <iostream>
#include <vector>
// Include common routines
//#include <verilated.h>


# include <verilated_vcd_c.h>
//#include "dbus_helper.hpp"



template <class T>
class elastic {

public:
	T *top;
	vluint64_t* main_time;
	VerilatedVcdC* tfp = NULL;
	unsigned char *clk;
	unsigned int *t_data;
	unsigned char *t_valid,*t_ready;
	unsigned int *i_data;
	unsigned char *i_valid,*i_ready;

	// testing another input
	unsigned int *t1_data;
	unsigned char *t1_valid,*t1_ready;


	elastic(T *top, vluint64_t *main_time, VerilatedVcdC *tfp){
		this->top=top;
		this->main_time=main_time;
		this->tfp=tfp;
		clk=&(top->clk);
		t_data=&(top->t0_data);
		t_valid=&(top->t0_valid);
		t_ready=&(top->t0_ready);
		i_data=&(top->i0_data);
		i_valid=&(top->i0_valid);
		i_ready=&(top->i0_ready);
	}

	elastic(
			T *top,
			vluint64_t *main_time,
			VerilatedVcdC *tfp,
			unsigned char *clk,
			unsigned char *t_valid,
			unsigned char *t_ready,
			unsigned int *t_data,
			unsigned char *i_valid,
			unsigned char *i_ready,
			unsigned int *i_data
	){
		this->top=top;
		this->main_time=main_time;
		this->clk=clk;
		this->tfp=tfp;
		this->t_data=t_data;
		this->t_valid=t_valid;
		this->t_ready=t_ready;
		this->i_data=i_data;
		this->i_valid=i_valid;
//		printf ("I am in elastic constructor\n");
		this->i_ready=i_ready;
	}

	/*void elastic_send(std::vector<int> din){
		for(std::vector<int>::iterator it=din.begin(); it!=din.end(); ++it){
			(*main_time)++;
			*clk = !*clk;
		}
	}

	std::vector<int> elastic_receive(){
		std::vector<int> dout;
		(*main_time)++;
		*clk = !*clk;
		return dout;
	}*/

	std::vector<int> data_txrx(dbus<T>* dbus_inst, int num_output, int verbose_output, std::vector<int> din, bool randValid, bool randReady){
		std::vector<int> dout;
		int valid_output = 0;
		int total_clocks = 0;
		*(i_ready)=1;
		bool init = true;

//		printf ("I am in data_txrx\n");
		std::vector<int>::iterator it = din.begin();
//		VL_PRINTF("init.............\n");

		while(it!=din.end()) {

			dbus_inst->Tick();
			total_clocks++;


			//capture output dataelastic/sim/
			if(*(i_valid) && *(i_ready)){
				dout.push_back(*(i_data));
				valid_output++;
			}

			if(randReady && rand()%3) {
				*(i_ready)=!*(i_ready);
			}

			if(init) { //init transaction
				*(t_data)=*it;
				*(t_valid)=1;
				init=false;
				it++;
			}
			else {
				if(*(t_ready)) {
					*(t_data)=*it;

					*(t_valid)=(!randValid)?1:
							(randValid && rand()%2)?1:0;
					if(*(t_valid))
						it++;
				} else if (!randValid) {
					it++;
				}
			}

//			top->eval();

//			if(tfp) {tfp->dump(*main_time);}


		}
		*(t_valid)=0;

		//capture any remaining outputs
		for(int i = 0; valid_output != num_output; i++) {
			//			(*main_time)++;
			//			*clk = !*clk;

			dbus_inst->Tick();
			total_clocks++;

			//capture output data
			if(*(i_valid) & *(i_ready)){
				dout.push_back(*(i_data));
				valid_output++;
			}
			if(randReady && rand()%3)
				*(i_ready)=!*(i_ready);

//			top->eval();
//			if(tfp) {tfp->dump(*main_time);}



		}
		if (verbose_output) {
			std::cout << "Totoal clocks = " << verbose_output << "\n";
		}
		return dout;
	}
};
