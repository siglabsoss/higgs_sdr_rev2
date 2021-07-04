#include "signals.h"
#include <verilated.h>
#include <sys/stat.h>

// Include model header, generated from Verilating "top.v"
#include "Vtb_higgs_top.h"

// If "verilator --trace" is used, include the tracing class
//#if VM_TRACE
# include <verilated_vcd_c.h>
//#endif

// // Current simulation time (64-bit unsigned)
// vluint64_t main_time = 0;
// // Called by $time in Verilog
// double sc_time_stamp() {
//     return main_time;  // Note does conversion to real, to match SystemC
// }
//

union WideSignal {
  uint32_t* sig32;
  uint64_t* sig64;
  };


Vtb_higgs_top* top;
VerilatedVcdC* tfp;
void signals::init_top() {

  //Verilated::debug(0);
  // Set debug level, 0 is off, 9 is highest presently used
  Verilated::debug(0);
  // Randomization reset policy
  Verilated::randReset(2);

  // Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
  top = new Vtb_higgs_top; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  //#if VM_TRACE
  // If verilator was invoked with --trace argument,
  // and if at run time passed the +trace argument, turn on tracing
  tfp = NULL;
  //  const char* flag = Verilated::commandArgsPlusMatch("trace");
  //  if (flag && 0==strcmp(flag, "+trace")) {
    Verilated::traceEverOn(true);  // Verilator must compute traced signals
    //VL_PRINTF("Enabling waves into logs/vlt_dump.vcd...\n");
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);  // Trace 99 levels of hierarchy
    Verilated::mkdir("logs");

    //mkdir("logs",0x775);

    tfp->open("logs/vlt_dump.vcd");  // Open the dump file
    //  }
  //#endif

  // Set some inputs
}

  
      
	
uint32_t signals::clk(uint32_t val) {
  top->clk = val;
  return top->clk;
}
	
      
  
      
	
uint32_t signals::MIB_MASTER_RESET(uint32_t val) {
  top->MIB_MASTER_RESET = val;
  return top->MIB_MASTER_RESET;
}
	
      
  
      
	
uint32_t signals::i_data_adc(uint32_t val) {
  top->i_data_adc = val;
  return top->i_data_adc;
}
	
      
  
      
	
uint32_t signals::i_data_valid_adc(uint32_t val) {
  top->i_data_valid_adc = val;
  return top->i_data_valid_adc;
}
	
      
  
      
	
uint32_t signals::tx_turnstile_data_in(uint32_t val) {
  top->tx_turnstile_data_in = val;
  return top->tx_turnstile_data_in;
}
	
      
  
      
	
uint32_t signals::tx_turnstile_data_valid(uint32_t val) {
  top->tx_turnstile_data_valid = val;
  return top->tx_turnstile_data_valid;
}
	
      
  
      
	
uint32_t signals::tx_turnstile_data_ready() {
  return top->tx_turnstile_data_ready;
}
	
      
  
      
	
uint32_t signals::ringbus_in_data(uint32_t val) {
  top->ringbus_in_data = val;
  return top->ringbus_in_data;
}
	
      
  
      
	
uint32_t signals::ringbus_in_data_vld(uint32_t val) {
  top->ringbus_in_data_vld = val;
  return top->ringbus_in_data_vld;
}
	
      
  
      
	
uint32_t signals::ringbus_in_data_ready() {
  return top->ringbus_in_data_ready;
}
	
      
  
      
	
uint32_t signals::ringbus_out_data() {
  return top->ringbus_out_data;
}
	
      
  
      
	
uint32_t signals::ringbus_out_data_vld() {
  return top->ringbus_out_data_vld;
}
	
      
  
      
	
uint32_t signals::ring_bus_i0_ready(uint32_t val) {
  top->ring_bus_i0_ready = val;
  return top->ring_bus_i0_ready;
}
	
      
  
      
	
uint32_t signals::o_data_valid_dac() {
  return top->o_data_valid_dac;
}
	
      
  
      
	
uint32_t signals::i_o_ready_dac() {
  return top->i_o_ready_dac;
}
	
      
  
      
	
uint32_t signals::o_data_dac() {
  return top->o_data_dac;
}
	
      
  
      
	
uint32_t signals::snap_cs20_riscv_out_data() {
  return top->snap_cs20_riscv_out_data;
}
	
      
  
      
	
uint32_t signals::snap_cs20_riscv_out_valid() {
  return top->snap_cs20_riscv_out_valid;
}
	
      
  
      
	
uint32_t signals::snap_cs20_riscv_out_ready() {
  return top->snap_cs20_riscv_out_ready;
}
	
      
  
      
	
uint32_t signals::adc_data_out(uint32_t val) {
  top->adc_data_out = val;
  return top->adc_data_out;
}
	
      
  
      
	
uint32_t signals::adc_data_out_valid(uint32_t val) {
  top->adc_data_out_valid = val;
  return top->adc_data_out_valid;
}
	
      
  
      
	
uint32_t signals::adc_data_out_ready() {
  return top->adc_data_out_ready;
}
	
      
  
      
	
uint32_t signals::o_data_eth() {
  return top->o_data_eth;
}
	
      
  
      
	
uint32_t signals::o_data_valid_eth() {
  return top->o_data_valid_eth;
}
	
      
  
      
	
uint32_t signals::DAC_CTRL_SDIO() {
  return top->DAC_CTRL_SDIO;
}
	
      
  
      
	
uint32_t signals::DAC_CTRL_SDENN() {
  return top->DAC_CTRL_SDENN;
}
	
      
  
      
	
uint32_t signals::DAC_CTRL_SCLK() {
  return top->DAC_CTRL_SCLK;
}
	
      
  
      
	
uint32_t signals::DAC_CTRL_RESETN() {
  return top->DAC_CTRL_RESETN;
}
	
      
  


int signals::eval() {
  static vluint64_t main_time = 0;
  main_time++;
  top->eval();
  tfp->dump(main_time);
  // Read outputs
  /*  VL_PRINTF ("[%" VL_PRI64 "d] clk=%x rstl=%x iquad=%" VL_PRI64 "x"
	     " -> oquad=%" VL_PRI64"x owide=%x_%08x_%08x\n",
	     main_time, top->clk, top->reset_l, top->in_quad,
	     top->out_quad, top->out_wide[2], top->out_wide[1], top->out_wide[0]);*/
  return 0;
}

int signals::finish() {
  top->final();
  tfp->close();
  tfp = NULL;
  return 0;
}


    
      
Napi::Number signals::clkWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::clk(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->clk);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::MIB_MASTER_RESETWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::MIB_MASTER_RESET(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->MIB_MASTER_RESET);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_data_adcWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::i_data_adc(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->i_data_adc);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_data_valid_adcWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::i_data_valid_adc(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->i_data_valid_adc);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::tx_turnstile_data_inWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::tx_turnstile_data_in(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->tx_turnstile_data_in);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::tx_turnstile_data_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::tx_turnstile_data_valid(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->tx_turnstile_data_valid);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::tx_turnstile_data_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->tx_turnstile_data_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::ringbus_in_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::ringbus_in_data(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->ringbus_in_data);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::ringbus_in_data_vldWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::ringbus_in_data_vld(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->ringbus_in_data_vld);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::ringbus_in_data_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->ringbus_in_data_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::ringbus_out_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->ringbus_out_data);
  return returnValue;
}
      
    

    
      
Napi::Number signals::ringbus_out_data_vldWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->ringbus_out_data_vld);
  return returnValue;
}
      
    

    
      
Napi::Number signals::ring_bus_i0_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::ring_bus_i0_ready(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->ring_bus_i0_ready);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::o_data_valid_dacWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->o_data_valid_dac);
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_o_ready_dacWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->i_o_ready_dac);
  return returnValue;
}
      
    

    
      
Napi::Number signals::o_data_dacWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->o_data_dac);
  return returnValue;
}
      
    

    
      
Napi::Number signals::snap_cs20_riscv_out_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->snap_cs20_riscv_out_data);
  return returnValue;
}
      
    

    
      
Napi::Number signals::snap_cs20_riscv_out_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->snap_cs20_riscv_out_valid);
  return returnValue;
}
      
    

    
      
Napi::Number signals::snap_cs20_riscv_out_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->snap_cs20_riscv_out_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::adc_data_outWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::adc_data_out(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->adc_data_out);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::adc_data_out_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::adc_data_out_valid(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->adc_data_out_valid);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::adc_data_out_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->adc_data_out_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::o_data_ethWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->o_data_eth);
  return returnValue;
}
      
    

    
      
Napi::Number signals::o_data_valid_ethWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->o_data_valid_eth);
  return returnValue;
}
      
    

    
      
Napi::Number signals::DAC_CTRL_SDIOWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->DAC_CTRL_SDIO);
  return returnValue;
}
      
    

    
      
Napi::Number signals::DAC_CTRL_SDENNWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->DAC_CTRL_SDENN);
  return returnValue;
}
      
    

    
      
Napi::Number signals::DAC_CTRL_SCLKWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->DAC_CTRL_SCLK);
  return returnValue;
}
      
    

    
      
Napi::Number signals::DAC_CTRL_RESETNWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->DAC_CTRL_RESETN);
  return returnValue;
}
      
    




void signals::evalWrapped(const Napi::CallbackInfo& info) {
    signals::eval();
  // Napi::Env env = info.Env();

  // Napi::Number returnValue = Napi::Number::New(env, signals::eval());
  // return returnValue;
  
}

void signals::finishWrapped(const Napi::CallbackInfo& info) {
    signals::finish();
    // Napi::Env env = info.Env();

    // Napi::Number returnValue = Napi::Number::New(env, signals::finish());
    // return returnValue;
}

void signals::initWrapped(const Napi::CallbackInfo& info) {
    // Napi::Env env = info.Env();
    signals::init_top();
}

Napi::Object signals::Init(Napi::Env env, Napi::Object exports) {


    
  exports.Set("clk", Napi::Function::New(env, signals::clkWrapped));
  

    
  exports.Set("MIB_MASTER_RESET", Napi::Function::New(env, signals::MIB_MASTER_RESETWrapped));
  

    
  exports.Set("i_data_adc", Napi::Function::New(env, signals::i_data_adcWrapped));
  

    
  exports.Set("i_data_valid_adc", Napi::Function::New(env, signals::i_data_valid_adcWrapped));
  

    
  exports.Set("tx_turnstile_data_in", Napi::Function::New(env, signals::tx_turnstile_data_inWrapped));
  

    
  exports.Set("tx_turnstile_data_valid", Napi::Function::New(env, signals::tx_turnstile_data_validWrapped));
  

    
  exports.Set("tx_turnstile_data_ready", Napi::Function::New(env, signals::tx_turnstile_data_readyWrapped));
  

    
  exports.Set("ringbus_in_data", Napi::Function::New(env, signals::ringbus_in_dataWrapped));
  

    
  exports.Set("ringbus_in_data_vld", Napi::Function::New(env, signals::ringbus_in_data_vldWrapped));
  

    
  exports.Set("ringbus_in_data_ready", Napi::Function::New(env, signals::ringbus_in_data_readyWrapped));
  

    
  exports.Set("ringbus_out_data", Napi::Function::New(env, signals::ringbus_out_dataWrapped));
  

    
  exports.Set("ringbus_out_data_vld", Napi::Function::New(env, signals::ringbus_out_data_vldWrapped));
  

    
  exports.Set("ring_bus_i0_ready", Napi::Function::New(env, signals::ring_bus_i0_readyWrapped));
  

    
  exports.Set("o_data_valid_dac", Napi::Function::New(env, signals::o_data_valid_dacWrapped));
  

    
  exports.Set("i_o_ready_dac", Napi::Function::New(env, signals::i_o_ready_dacWrapped));
  

    
  exports.Set("o_data_dac", Napi::Function::New(env, signals::o_data_dacWrapped));
  

    
  exports.Set("snap_cs20_riscv_out_data", Napi::Function::New(env, signals::snap_cs20_riscv_out_dataWrapped));
  

    
  exports.Set("snap_cs20_riscv_out_valid", Napi::Function::New(env, signals::snap_cs20_riscv_out_validWrapped));
  

    
  exports.Set("snap_cs20_riscv_out_ready", Napi::Function::New(env, signals::snap_cs20_riscv_out_readyWrapped));
  

    
  exports.Set("adc_data_out", Napi::Function::New(env, signals::adc_data_outWrapped));
  

    
  exports.Set("adc_data_out_valid", Napi::Function::New(env, signals::adc_data_out_validWrapped));
  

    
  exports.Set("adc_data_out_ready", Napi::Function::New(env, signals::adc_data_out_readyWrapped));
  

    
  exports.Set("o_data_eth", Napi::Function::New(env, signals::o_data_ethWrapped));
  

    
  exports.Set("o_data_valid_eth", Napi::Function::New(env, signals::o_data_valid_ethWrapped));
  

    
  exports.Set("DAC_CTRL_SDIO", Napi::Function::New(env, signals::DAC_CTRL_SDIOWrapped));
  

    
  exports.Set("DAC_CTRL_SDENN", Napi::Function::New(env, signals::DAC_CTRL_SDENNWrapped));
  

    
  exports.Set("DAC_CTRL_SCLK", Napi::Function::New(env, signals::DAC_CTRL_SCLKWrapped));
  

    
  exports.Set("DAC_CTRL_RESETN", Napi::Function::New(env, signals::DAC_CTRL_RESETNWrapped));
  

  exports.Set("eval", Napi::Function::New(env, signals::evalWrapped));
  exports.Set("finish", Napi::Function::New(env, signals::finishWrapped));
  exports.Set("init", Napi::Function::New(env, signals::initWrapped));
  
  return exports;
}

