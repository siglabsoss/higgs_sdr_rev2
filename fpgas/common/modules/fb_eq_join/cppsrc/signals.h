#include "napi.h"
#include <string>

namespace signals {

    void init_top(std::string name);
  
  int tick();
  
      
	
  uint32_t clk(uint32_t val);
	
      
  
      
	
  uint32_t t_fb_valid(uint32_t val);
	
      
  
      
	
  uint32_t t_fb_last(uint32_t val);
	
      
  
      
	
  uint32_t t_fb_ready();
	
      
  
      
	
  uint32_t t_eq_valid(uint32_t val);
	
      
  
      
	
  uint32_t t_eq_last(uint32_t val);
	
      
  
      
	
  uint32_t t_eq_ready();
	
      
  
      
	
  uint32_t i_valid();
	
      
  
      
	
  uint32_t i_last();
	
      
  
      
	
  uint32_t i_ready(uint32_t val);
	
      
  
      
	
  uint32_t rstf(uint32_t val);
	
      
  
      
	
  uint32_t t_fb_data(uint32_t val);
	
      
  
      
	
  uint32_t t_eq_data(uint32_t val);
	
      
  
      
	
  uint32_t i_data();
	
      
  
  int eval();
  int finish();
  
  Napi::Number TickWrapped(const Napi::CallbackInfo& info);
  
      
  Napi::Number clkWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_fb_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_fb_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_fb_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_eq_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_eq_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_eq_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_lastWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number rstfWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_fb_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number t_eq_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_dataWrapped(const Napi::CallbackInfo& info);
      
  
  void evalWrapped(const Napi::CallbackInfo& info);
  void finishWrapped(const Napi::CallbackInfo& info);
  void initWrapped(const Napi::CallbackInfo& info);
  Napi::Object Init(Napi::Env env, Napi::Object exports);
};

