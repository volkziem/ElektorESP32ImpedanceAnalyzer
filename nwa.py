# nwa.py, V. Ziemann, 230105
import serial,time
import numpy as np
import matplotlib as mpl  # uncomment if plot does not show
mpl.use("TkAgg")          # uncomment if plot does not show
import matplotlib.pyplot as plt
s=serial.Serial("/dev/ttyUSB0",115200,timeout=1)
time.sleep(3)
s.flush()                  # clean input fom startup messages
s.write(b"SCALE 1\n")      # DAC amplitude 1/2 scale
s.write(b"FMIN 10000\n")   # minimum frequency
s.write(b"FMAX 50000\n")   # maximum frequency
s.write(b"FSTEP 1000\n")   # frequency step size
s.write(b"STATE?\n")       # Retrieve settings
reply=s.readline().decode('utf-8')
print(reply)
p=reply.split()  # p[0]="STATE"
nsteps=int((int(p[2])-int(p[1]))/int(p[3]))
print("nsteps = ",nsteps)
s.write(b"SWEEP?\n")       # initiate frequency sweep
xx=[]
yy=[]
for k in range(nsteps):
   # print(s.readline().decode('utf-8').strip())
   xx.append((int(p[1])+(k-1)*int(p[3]))/1e3)
   yy.append(s.readline().decode('utf-8').strip())
Rcal=1   # calibration resistor in kOhm
s.write(b"GETCALIB?\n")
reply=s.readline().decode('utf-8')
print(reply)
tmp=reply.split()
cal=[float(tmp[1]),float(tmp[2])]
print(cal)
s.close()
xx=list(map(float,xx))  # convert to integer array
yy=list(map(float,yy))
Zabs=Rcal*np.divide(np.polyval(cal,xx),yy)
plt.subplot(2,1,1)
plt.plot(xx,yy,'b',xx,np.polyval(cal,xx),'r')
plt.xlabel("f [kHz]")
plt.ylabel("raw")
plt.subplot(2,1,2)
plt.plot(xx,Zabs,'b')
plt.xlabel("f [kHz]")
plt.ylabel("Impedance [kOhm]")
plt.show()

#print("resistance_kOhm = ",np.mean(Zabs))

tmp=2e6*np.pi*np.multiply(Zabs,xx)
print("capacitance_nF = ",np.mean(np.divide(np.ones(nsteps),tmp))*1e9)

#tmp=np.divide(Zabs,xx)/(2*np.pi)
#print("inductance_mH = ",np.mean(tmp)*1e3)







