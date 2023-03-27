// esp32_impedance_analyzer, V. Ziemann, 230104

#include <SPIFFS.h>
#include "ESP32_I2Sconfig.h"
char line[30];
uint32_t i2s_sample_rate=1000000;
size_t bytes_read;
uint16_t buffer[1024]={0}, swap;
float freqmin=10000,freqmax=50000,freqstep=1000;
float slope=0,offset=0,calib_slope=0,calib_offset=0;

void setup() {
  Serial.begin(115200); delay(1000);
  i2sInit(i2s_sample_rate);
  dac25freq=5000;
  dac25scale=1;
  cwDACinit(dac25freq,dac25scale,0);
  if (!SPIFFS.begin()) {
    Serial.println("ERROR: no SPIFFS filesystem found");
  } else {
    File f=SPIFFS.open("/saved_calib.dat","r");
    if (f) {    
      f.readStringUntil('\n').toCharArray(line,80);
      sscanf(line,"%g %g",&calib_slope,&calib_offset);
      Serial.printf("Calibration data: %g %g\n",calib_slope,calib_offset);
    } else {
      Serial.println("ERROR: No calibration data");
    }
  }
  Serial.println("Setup complete");  
}

void loop() {
  if (Serial.available()) {
    Serial.readStringUntil('\n').toCharArray(line,30);
    if (strstr(line,"WF?")==line) { 
      i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 15);  // read i2s-adc
      for (int j=0; j<bytes_read/2; j+=2) {  // left and right channel come in wrong temporal order
        swap=buffer[j+1]; buffer[j+1]=buffer[j]; buffer[j]=swap;
      }   
      for (int k=0;k<bytes_read/2;k++) {Serial.println(buffer[k] & 0x0FFF);}
    } else if (strstr(line,"FREQ")==line) {
      dac25freq=atof(&line[5]);
      cwDACinit(dac25freq,dac25scale,0);
    } else if (strstr(line,"SCALE")==line) {
      dac25scale=atof(&line[6]);
      cwDACinit(dac25freq,dac25scale,0);  
    } else if (strstr(line,"SAMP")==line) {
      i2s_sample_rate=(uint32_t)atof(&line[5]);
      i2s_adc_disable(I2S_NUM_0); 
      i2s_driver_uninstall(I2S_NUM_0);
      i2sInit(i2s_sample_rate);
      cwDACinit(dac25freq,dac25scale,0); 
    } else if (strstr(line,"FMIN")==line) {freqmin=atof(&line[5]);
    } else if (strstr(line,"FMAX")==line) {freqmax=atof(&line[5]);
    } else if (strstr(line,"FSTEP")==line) {freqstep=atof(&line[6]);  
    } else if (strstr(line,"STATE?")==line) {
      Serial.printf("STATE %g %g %g\n",freqmin,freqmax,freqstep);
    } else if (strstr(line,"SAVECALIB")==line) {  // save calibration data
      File f=SPIFFS.open("/saved_calib.dat","w"); 
      f.printf(" %g %g\n",slope,offset);
      f.close(); 
      calib_slope=slope; calib_offset=offset;
    } else if (strstr(line,"GETCALIB?")==line) {  // get calibration data  
      Serial.printf("GETCALIB %g %g\n",calib_slope,calib_offset); 
    } else if (strstr(line,"SWEEP?")==line) {   
      int bmin=10000,bmax=0,nn=0;
      float q1=0,q2=0,S0,S1,S2;
      for (float f=freqmin; f<freqmax; f+=freqstep) {
        nn++;
        dac25freq=f;
        cwDACinit(dac25freq,dac25scale,0); 
        delay(50);
        i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 15);  
        bmin=10000; bmax=0;
        for (int k=0;k<bytes_read/2;k++) {
           bmin=min(bmin,buffer[k] & 0x0FFF);
           bmax=max(bmax,buffer[k] & 0x0FFF);
        }  
        Serial.println(bmax-bmin);
        q1+=nn*(bmax-bmin);  // for the straight-line fit
        q2+=(bmax-bmin);
      }
      S0=nn; S1=0.5*nn*(nn+1); S2=nn*(nn+1)*(2*nn+1)/6.0;  // appendix B
      slope=(S0*q1-S1*q2)/(S0*S2-S1*S1);        // slope in integer steps
      offset=(-S1*q1+S2*q2)/(S0*S2-S1*S1);      // and offset: y=slope*nn+offset 
      offset=offset+slope*(1-freqmin/freqstep); // convert to frequencies in kHz
      slope=1e3*slope/freqstep;                 // with: f=fmin+(nn-1)*df
    }
  }
  yield();
}
