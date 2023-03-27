// ESP32_I2Sconfig.j, V. Ziemann, 230104
#include <driver/i2s.h>
bool i2s_adc_enabled=false;
void i2sInit(uint32_t i2s_sample_rate){
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate =  i2s_sample_rate,                 // The format of the signal using ADC_BUILT_IN
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,    // is fixed at 12bit, stereo, MSB    
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,     // requires sample reordering
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,     
    .dma_buf_len = 1024,    // 1024 is maximum length 
    .use_apll = true,       // must be true for low frequencies to work
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  Serial.printf("Attempting to setup I2S ADC with sampling frequency %d Hz\n", i2s_sample_rate);
  if(ESP_OK != i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL)){
    Serial.printf("Error installing I2S."); 
  }
  if(ESP_OK != i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4)){    // pin 32
    Serial.printf("Error setting up ADC.");
  }
  if(ESP_OK != adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11)){
    Serial.printf("Error setting up ADC attenuation.");
  }

  if(ESP_OK != i2s_adc_enable(I2S_NUM_0)){
    Serial.printf("Error enabling ADC.");
  }
//  Serial.printf("I2S ADC setup ok\n");
}

#include <driver/dac.h>
#include <soc/sens_reg.h>
#include "soc/rtc.h"
float dac25freq=0;
int dac25scale=0;   // output full scale
// from https://github.com/krzychb/dac-cosine
void cwDACinit(float freq, int scale, int offset) { //.........................cwDACinit
  if (abs(freq) < 1e-3) {dac_output_disable(DAC_CHANNEL_1); return;}
  int frequency_step=max(1.0,floor(0.5+0.95*65536.0*freq/RTC_FAST_CLK_FREQ_APPROX)); 
  SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
  SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV1, 2, SENS_DAC_INV1_S);
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL1_REG, SENS_SW_FSTEP, frequency_step, SENS_SW_FSTEP_S);
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_SCALE1, scale, SENS_DAC_SCALE1_S);
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_DC1, offset, SENS_DAC_DC1_S);
  dac_output_enable(DAC_CHANNEL_1);
//  float frequency = RTC_FAST_CLK_FREQ_APPROX  * (float) frequency_step / 65536;
//  Serial.printf("DAC config: clk_8m_div: 0, frequency step: %d, frequency: %.0f Hz\n", frequency_step, frequency);
}
