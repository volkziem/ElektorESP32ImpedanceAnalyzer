% nwa.m, V. Ziemann, 230105
clear all; close all; 
s=serialport('/dev/ttyUSB0',115200);
pause(3)
flush(s)
write(s,"SCALE 1\n"); pause(0.1);
write(s,"FMIN 5000\n"); pause(0.1);
write(s,"FMAX 20000\n"); pause(0.1);
write(s,"FSTEP 1000\n"); pause(0.1);
write(s,"STATE?\n"); pause(0.1); reply=serialReadline(s);
p=sscanf(reply,"STATE %g %g %g");
nsteps=(p(2)-p(1))/p(3);
write(s,"SWEEP?\n"); pause(0.1);
xx=zeros(nsteps,1); yy=xx;
for k=1:nsteps
  xx(k)=(p(1)+(k-1)*p(3))/1e3;
  yy(k)=str2double(serialReadline(s));
end
Rcal=1;                              % calibration resistor in kOhm
%write(s,"SAVECALIB\n"); pause(0.1); % uncomment if calib resistor is in
write(s,"GETCALIB?\n"); pause(0.1); reply=serialReadline(s);
cal=sscanf(reply,"GETCALIB %g %g")
clear s
subplot(2,1,1); plot(xx,yy,'b',xx,polyval(cal,xx),'r');
xlim([min(xx),max(xx)])
xlabel('f [kHz]'); ylabel('raw [ADC counts]')
Zabs=Rcal*polyval(cal,xx)./yy;
subplot(2,1,2); plot(xx,Zabs,'b'); xlim([min(xx),max(xx)])
xlabel('f [kHz]'); ylabel('Impedance [k\Omega]')

%capacitance_nF=mean(1./(2*pi*Zabs*1e3.*xx*1e3))*1e9
%resistance_kOhm=mean(Zabs)
%inductance_mH=mean(1e3*Zabs./(2*pi*xx*1e3))*1e3
