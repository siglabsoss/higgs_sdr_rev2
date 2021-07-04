#include "napi.h"


namespace signals {

  void init_top();
  
  int tick();
  
      
	
  uint32_t clk(uint32_t val);
	
      
  
      
	
  uint32_t MIB_MASTER_RESET(uint32_t val);
	
      
  
      
	
  uint32_t i_data_adc(uint32_t val);
	
      
  
      
	
  uint32_t i_data_valid_adc(uint32_t val);
	
      
  
      
	
  uint32_t tx_turnstile_data_in(uint32_t val);
	
      
  
      
	
  uint32_t tx_turnstile_data_valid(uint32_t val);
	
      
  
      
	
  uint32_t tx_turnstile_data_ready();
	
      
  
      
	
  uint32_t ringbus_in_data(uint32_t val);
	
      
  
      
	
  uint32_t ringbus_in_data_vld(uint32_t val);
	
      
  
      
	
  uint32_t ringbus_in_data_ready();
	
      
  
      
	
  uint32_t ringbus_out_data();
	
      
  
      
	
  uint32_t ringbus_out_data_vld();
	
      
  
      
	
  uint32_t ring_bus_i0_ready(uint32_t val);
	
      
  
      
	
  uint32_t o_data_valid_dac();
	
      
  
      
	
  uint32_t i_o_ready_dac();
	
      
  
      
	
  uint32_t o_data_dac();
	
      
  
      
	
  uint32_t snap_cs20_riscv_out_data();
	
      
  
      
	
  uint32_t snap_cs20_riscv_out_valid();
	
      
  
      
	
  uint32_t snap_cs20_riscv_out_ready();
	
      
  
      
	
  uint32_t adc_data_out(uint32_t val);
	
      
  
      
	
  uint32_t adc_data_out_valid(uint32_t val);
	
      
  
      
	
  uint32_t adc_data_out_ready();
	
      
  
      
	
  uint32_t o_data_eth();
	
      
  
      
	
  uint32_t o_data_valid_eth();
	
      
  
      
	
  uint32_t DAC_CTRL_SDIO();
	
      
  
      
	
  uint32_t DAC_CTRL_SDENN();
	
      
  
      
	
  uint32_t DAC_CTRL_SCLK();
	
      
  
      
	
  uint32_t DAC_CTRL_RESETN();
	
      
  
  int eval();
  int finish();
  
  Napi::Number TickWrapped(const Napi::CallbackInfo& info);
  
      
  Napi::Number clkWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number MIB_MASTER_RESETWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_data_adcWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_data_valid_adcWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number tx_turnstile_data_inWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number tx_turnstile_data_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number tx_turnstile_data_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ringbus_in_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ringbus_in_data_vldWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ringbus_in_data_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ringbus_out_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ringbus_out_data_vldWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number ring_bus_i0_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number o_data_valid_dacWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number i_o_ready_dacWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number o_data_dacWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number snap_cs20_riscv_out_dataWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number snap_cs20_riscv_out_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number snap_cs20_riscv_out_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number adc_data_outWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number adc_data_out_validWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number adc_data_out_readyWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number o_data_ethWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number o_data_valid_ethWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number DAC_CTRL_SDIOWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number DAC_CTRL_SDENNWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number DAC_CTRL_SCLKWrapped(const Napi::CallbackInfo& info);
      
  
      
  Napi::Number DAC_CTRL_RESETNWrapped(const Napi::CallbackInfo& info);
      
  
  void evalWrapped(const Napi::CallbackInfo& info);
  void finishWrapped(const Napi::CallbackInfo& info);
  void initWrapped(const Napi::CallbackInfo& info);
  Napi::Object Init(Napi::Env env, Napi::Object exports);
};
