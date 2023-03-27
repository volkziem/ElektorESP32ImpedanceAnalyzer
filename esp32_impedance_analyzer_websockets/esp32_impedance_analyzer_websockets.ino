// ESP32 impedance analyzer with websockets, V. Ziemann, 230108
// added to dish out web page stored on SPIFFS from http port 80

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

#include <WiFi.h>
#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(81);
#include <WebServer.h>
#include <SPIFFS.h>
WebServer server2(80);
#include <ArduinoJson.h>
#include "ESP32_I2Sconfig.h"
uint32_t i2s_sample_rate=1000000;
size_t bytes_read;
uint16_t buffer[1024]={0}, swap;
float freqmin=10000,freqmax=50000,freqstep=1000;
float slope=0,offset=0,calib_slope=0,calib_offset=0;
volatile uint8_t websock_num=0,info_available=0,output_ready=0;
volatile int mmode=-1;
char info_buffer[80];
char out[20000]; DynamicJsonDocument doc(20000);
float est_cap=0,est_res=0,est_ind=0;

const int npts=1024;
int sample_buffer[npts],sample_buffer2[npts],isamp=0;
bool sample_buffer_ready=false;

//...................................................................
void sendMSG(char *nam, const char *msg) {
  (void) sprintf(info_buffer,"{\"%s\":\"%s\"}",nam,msg);  
  info_available=1;
}

//...................................................................
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED: 
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", 
          num, ip[0], ip[1], ip[2], ip[3], payload);
        char msg[100]="Connected to sensor";
        char buf[20],out[100];  
      }
      sendMSG("INFO","ESP32: Successfully connected");
      break;
    case WStype_TEXT:
    {
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      //........................................parse JSON
      DynamicJsonDocument root(300);
      deserializeJson(root,payload);
      const char *cmd = root["cmd"];
      const long val = root["val"];
      if (strstr(cmd,"SWEEP")) { 
        sendMSG("INFO","ESP32: Received Sweep command");
        mmode=1;
      } else if (strstr(cmd,"SAVECALIB")) {
        sendMSG("INFO","ESP32: Received command to save calibration constants"); 
        mmode=2; 
      } else if (strstr(cmd,"SCALE")) {
        dac25scale=abs(val); Serial.printf("DAC scale parameter = %d\n",dac25scale);
        cwDACinit(dac25freq,dac25scale,0);  
      } else if (strstr(cmd,"SAMP")) {
        i2s_sample_rate=(uint32_t)val;
        Serial.printf("Sample rate = %d\n",i2s_sample_rate);  
        i2s_adc_disable(I2S_NUM_0); i2s_driver_uninstall(I2S_NUM_0);
        i2sInit(i2s_sample_rate);
        cwDACinit(dac25freq,dac25scale,0); 
      } else if (strstr(cmd,"FMIN")) {freqmin=val; Serial.printf("FreqMin = %g\n",freqmin);
      } else if (strstr(cmd,"FMAX")) {freqmax=val; Serial.printf("FreqMax = %g\n",freqmax);
      } else if (strstr(cmd,"FSTEP")) {freqstep=val; Serial.printf("FreqStep = %g\n",freqstep);            
      } else if (strstr(cmd,"MMODE")) {
        mmode=val;
      } else {
        Serial.println("Unknown command");
        sendMSG("INFO","ESP32: Unknown command received");
      }
      break;
    }
    case WStype_BIN:
      break;
    default:
      break;
  }
}
//..............................................................http server
void handle_notfound() {
  server2.send(404,"text/plain","not found, use http://ip-address/");
}
//..................................................................setup
void setup() {
  Serial.begin(115200);
  delay(1000);
  //..............................................client mode
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {Serial.print("."); delay(500);}
  Serial.print("\nConnected to ");  Serial.print(ssid);
  Serial.print(" with IP address: "); Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //.....................................................http server
  if (!SPIFFS.begin(true)) {
    Serial.println("no SPIFFS filesystem found");
    return;
  } else {
    Serial.println("SPIFFS file system found"); 
    server2.begin();          
    server2.serveStatic("/", SPIFFS, "/esp32_impedance_analyzer.html");
    server2.onNotFound(handle_notfound);
    Serial.println("HTTP server2 started on port 80"); 
    File f=SPIFFS.open("/saved_calib.dat","r");
    if (f) { 
      char line[30];   
      f.readStringUntil('\n').toCharArray(line,80);
      sscanf(line,"%g %g",&calib_slope,&calib_offset);
      Serial.printf("Calibration data: %g %g\n",calib_slope,calib_offset);
    } else {
      Serial.println("ERROR: No calibration data");
    }    
  }
  i2sInit(i2s_sample_rate);
  dac25freq=5000;
  dac25scale=1;
  cwDACinit(dac25freq,dac25scale,0);
}
//.................................................................loop
void loop() { 
  server2.handleClient();    //..........................http server
  webSocket.loop();
  if (info_available==1) {
    info_available=0;
    webSocket.sendTXT(websock_num,info_buffer,strlen(info_buffer));
  }
  if (mmode==1) {
    mmode=-1;
    doc.to<JsonObject>();  // clear doc
    int bmin=10000,bmax=0,nn=0;
    float q1=0,q2=0,S0,S1,S2;
    est_cap=0; est_res=0; est_ind=0;
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
      float Z=(calib_slope*1e-3*f+calib_offset)/(bmax-bmin);
      est_cap+=1e9/(6.2832*Z*1e3*f);
      est_res+=Z;
      est_ind+=1e6*Z/(6.2832*f);
      doc["WF0"][nn-1]=floor((512/16)*Z);
      q1+=nn*(bmax-bmin);  // for the straight-line fit
      q2+=(bmax-bmin);
    }
    est_cap/=nn; est_res/=nn; est_ind/=nn;
    Serial.printf("Estimated capacitance = %g nF\n",est_cap);
    Serial.printf("Estimated resistance = %g kOhm\n",est_res);
    Serial.printf("Estimated inductance = %g mH\n",est_ind); 
    serializeJson(doc,out);
    webSocket.sendTXT(websock_num,out,strlen(out));  
    S0=nn; S1=0.5*nn*(nn+1); S2=nn*(nn+1)*(2*nn+1)/6.0;  // appendix B
    slope=(S0*q1-S1*q2)/(S0*S2-S1*S1);        // slope in integer steps
    offset=(-S1*q1+S2*q2)/(S0*S2-S1*S1);      // and offset: y=slope*nn+offset 
    offset=offset+slope*(1-freqmin/freqstep); // convert to frequencies in kHz
    slope=1e3*slope/freqstep;                 // with: f=fmin+(nn-1)*df
    Serial.printf("Calib: %g   %g\n",slope,offset);
    sendMSG("INFO","ESP32: Sweep completed");    
  } else if (mmode==2) {
    mmode=-1;
    File f=SPIFFS.open("/saved_calib.dat","w"); 
    f.printf(" %g %g\n",slope,offset);
    f.close(); 
    calib_slope=slope; calib_offset=offset;
    Serial.println("Calibration parameters saved");
  } else if (mmode==100) {
    sprintf(out,"ESP32: Estimated capacitance = %.3g nF",est_cap);
    sendMSG("INFO",out);
  } else if (mmode==101) {
    sprintf(out,"ESP32: Estimated resistance = %.3g kOhm",est_res);
    sendMSG("INFO",out);
  } else if (mmode==102) {
    sprintf(out,"ESP32: Estimated inductance = %.3g mH",est_ind);
    sendMSG("INFO",out);  
  }
  yield();
}
