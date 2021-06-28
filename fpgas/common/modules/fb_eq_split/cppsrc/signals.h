#include "napi.h"
#include <string>

namespace signals {

    void init_top(std::string name);
  
  int tick();
  
      
	
  uint32_t clk(uint32_t val);
	
      
  
      
	
  uint32_t t_valid(uint32_t val);
	
      
  
      
	
  uint32_t t_ready();
	
      
  
      
	
  uint32_t i_fb_valid();
	
      
  
      
	
  uint32_t i_fb_last();
	
      
  
      
	
  uint32_t i_fb_ready(uint32_t val);
	
      
  
      
	
  uint32_t i_eq_valid();
	
      
  
      
	
  uint32_t i_eq_last();
	
      
  
      
	
  uint32_t i_eq_ready(uint32_t val);
	
      
  
      
	
  uint32_t rstf(uint32_t val);
	
      
  
      
	
  uint32_t t_data(uint32_t val);
	
      
  
      
	
  uint32_t i_fb_data();
	
      
  
      
	
  uint32_t i_eq_data();
	
      
  
  int eval();
  int finish();
  
  Napi::Number TickWrapped(const Napi::CallbackInfo& info);
  
      
  Napi::Number clkWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_fb_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_fb_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_fb_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_eq_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_eq_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_eq_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number rstfWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_fb_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_eq_dataWrapped(const Napi::CallbackInfo& info);
      
  
  void evalWrapped(const Napi::CallbackInfo& info);
  void finishWrapped(const Napi::CallbackInfo& info);
  void initWrapped(const Napi::CallbackInfo& info);
  Napi::Object Init(Napi::Env env, Napi::Object exports);
};

