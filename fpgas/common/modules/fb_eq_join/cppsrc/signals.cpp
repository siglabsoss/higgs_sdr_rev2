#include "signals.h"
#include <verilated.h>
#include <sys/stat.h>
#include <string>

// Include model header, generated from Verilating "top.v"
#include "Vfb_eq_join.h"

// If "verilator --trace" is used, include the tracing class
//#if VM_TRACE

# include <verilated_vcd_c.h>
VerilatedVcdC* tfp;

//#endif

// // Current simulation time (64-bit unsigned)
 vluint64_t main_time = 0;
// // Called by $time in Verilog
 double sc_time_stamp() {
     return main_time;  // Note does conversion to real, to match SystemC
 }
//

union WideSignal {
  uint32_t* sig32;
  uint64_t* sig64;
  };


Vfb_eq_join* top;
void signals::init_top(std::string name) {

  //Verilated::debug(0);
  // Set debug level, 0 is off, 9 is highest presently used
  Verilated::debug(0);
  // Randomization reset policy
  Verilated::randReset(2);

  // Construct the Verilated model, from Vtop.h generated from Verilating "top.v"
  top = new Vfb_eq_join; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

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

	std::string f = "logs/" + name + ".vcd";

       tfp->open(f.c_str());

    //  }
  //#endif

  // Set some inputs
}

  
      
	
uint32_t signals::clk(uint32_t val) {
  top->clk = val;
  return top->clk;
}
	
      
  
      
	
uint32_t signals::t_fb_valid(uint32_t val) {
  top->t_fb_valid = val;
  return top->t_fb_valid;
}
	
      
  
      
	
uint32_t signals::t_fb_last(uint32_t val) {
  top->t_fb_last = val;
  return top->t_fb_last;
}
	
      
  
      
	
uint32_t signals::t_fb_ready() {
  return top->t_fb_ready;
}
	
      
  
      
	
uint32_t signals::t_eq_valid(uint32_t val) {
  top->t_eq_valid = val;
  return top->t_eq_valid;
}
	
      
  
      
	
uint32_t signals::t_eq_last(uint32_t val) {
  top->t_eq_last = val;
  return top->t_eq_last;
}
	
      
  
      
	
uint32_t signals::t_eq_ready() {
  return top->t_eq_ready;
}
	
      
  
      
	
uint32_t signals::i_valid() {
  return top->i_valid;
}
	
      
  
      
	
uint32_t signals::i_last() {
  return top->i_last;
}
	
      
  
      
	
uint32_t signals::i_ready(uint32_t val) {
  top->i_ready = val;
  return top->i_ready;
}
	
      
  
      
	
uint32_t signals::rstf(uint32_t val) {
  top->rstf = val;
  return top->rstf;
}
	
      
  
      
	
uint32_t signals::t_fb_data(uint32_t val) {
  top->t_fb_data = val;
  return top->t_fb_data;
}
	
      
  
      
	
uint32_t signals::t_eq_data(uint32_t val) {
  top->t_eq_data = val;
  return top->t_eq_data;
}
	
      
  
      
	
uint32_t signals::i_data() {
  return top->i_data;
}
	
      
  


int signals::eval() {
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
      
    

    
      
Napi::Number signals::t_fb_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_fb_valid(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_fb_valid);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_fb_lastWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_fb_last(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_fb_last);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_fb_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->t_fb_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_eq_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_eq_valid(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_eq_valid);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_eq_lastWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_eq_last(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_eq_last);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_eq_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->t_eq_ready);
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_validWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->i_valid);
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_lastWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->i_last);
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_readyWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::i_ready(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->i_ready);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::rstfWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::rstf(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->rstf);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_fb_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_fb_data(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_fb_data);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::t_eq_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 1 || (info.Length() == 1 && !info[0].IsNumber())) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  if(info.Length() == 1) {
    Napi::Number val = info[0].As<Napi::Number>();
    returnValue = Napi::Number::New(env, signals::t_eq_data(val.Int32Value()));
  } else {
    returnValue = Napi::Number::New(env, top->t_eq_data);
  }
  return returnValue;
}
      
    

    
      
Napi::Number signals::i_dataWrapped(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(info.Length() > 0) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  }
    
  Napi::Number returnValue;
  returnValue = Napi::Number::New(env, top->i_data);
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
    std::string name = "waveform";
    Napi::Env env = info.Env();
    if(info.Length() > 0 ) {
	if(!info[0].IsString()) {
	    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
	}
	Napi::String val = info[0].As<Napi::String>();
	name = val.Utf8Value();
    }
    signals::init_top(name);
}

Napi::Object signals::Init(Napi::Env env, Napi::Object exports) {



  exports.Set("clk", Napi::Function::New(env, signals::clkWrapped));



  exports.Set("t_fb_valid", Napi::Function::New(env, signals::t_fb_validWrapped));



  exports.Set("t_fb_last", Napi::Function::New(env, signals::t_fb_lastWrapped));



  exports.Set("t_fb_ready", Napi::Function::New(env, signals::t_fb_readyWrapped));



  exports.Set("t_eq_valid", Napi::Function::New(env, signals::t_eq_validWrapped));



  exports.Set("t_eq_last", Napi::Function::New(env, signals::t_eq_lastWrapped));



  exports.Set("t_eq_ready", Napi::Function::New(env, signals::t_eq_readyWrapped));



  exports.Set("i_valid", Napi::Function::New(env, signals::i_validWrapped));



  exports.Set("i_last", Napi::Function::New(env, signals::i_lastWrapped));



  exports.Set("i_ready", Napi::Function::New(env, signals::i_readyWrapped));



  exports.Set("rstf", Napi::Function::New(env, signals::rstfWrapped));



  exports.Set("t_fb_data", Napi::Function::New(env, signals::t_fb_dataWrapped));



  exports.Set("t_eq_data", Napi::Function::New(env, signals::t_eq_dataWrapped));



  exports.Set("i_data", Napi::Function::New(env, signals::i_dataWrapped));


  exports.Set("eval", Napi::Function::New(env, signals::evalWrapped));
  exports.Set("finish", Napi::Function::New(env, signals::finishWrapped));
  exports.Set("init", Napi::Function::New(env, signals::initWrapped));
  return exports;

}
