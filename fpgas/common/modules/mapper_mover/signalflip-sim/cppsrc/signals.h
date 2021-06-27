#include "napi.h"


namespace signals {

  void init_top();
  
  int tick();
  
      
	
  uint32_t t_data(uint32_t val);
	
      
  
      
	
  uint32_t t_last(uint32_t val);
	
      
  
      
	
  uint32_t t_valid(uint32_t val);
	
      
  
      
	
  uint32_t t_ready();
	
      
  
      
	
  uint32_t i_data();
	
      
  
      
	
  uint32_t i_valid();
	
      
  
      
	
  uint32_t i_ready(uint32_t val);
	
      
  
      
	
  uint32_t clk(uint32_t val);
	
      
  
      
	
  uint32_t rstf(uint32_t val);
	
      
  
  int eval();
  int finish();
  
  Napi::Number TickWrapped(const Napi::CallbackInfo& info);
  
      
  Napi::Number t_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number clkWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number rstfWrapped(const Napi::CallbackInfo& info);
      
  
  void evalWrapped(const Napi::CallbackInfo& info);
  void finishWrapped(const Napi::CallbackInfo& info);
  void initWrapped(const Napi::CallbackInfo& info);
  Napi::Object Init(Napi::Env env, Napi::Object exports);
};
