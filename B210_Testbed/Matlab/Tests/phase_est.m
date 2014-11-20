close all
clear all
clc


N = 3e3;
phis_mdft = zeros(1,N);
phis_fft = zeros(1,N);

for(ii = 1:N)
    Fs = 40e3;
    Ts = 1/Fs;
    
    F0 = 1e3;
    Tsig = 3;
    
    t = 0:Ts:Tsig - Ts;
    
    phi1 = pi/3;
    phi2 = 0;
    
    
    m_settings = struct('fs',Fs,'f0',F0);
    
    
    sigma = 2;
    n1 = (sigma^2/2)*randn(1,length(t))+1j*(sigma^2/2)*randn(1,length(t));
    n2 = (sigma^2/2)*randn(1,length(t))+1j*(sigma^2/2)*randn(1,length(t));;
    
    
    x1 = (exp(1j*(2*pi*F0.*t + phi1))+n1);
    x2 = (exp(1j*(2*pi*F0.*t + phi2))+n2);
    
    
    F_axis = -Fs/2:Fs/2 - 1;
    Nf = length(F_axis);
    
    
    
    
    [phi_est delta_err] = mdtft(real(x1),real(x2),m_settings);
    
    phis_mdft(ii) = phi_est*180/pi;
    
    
    y1 = fftshift(fft(x1,Nf));
    y2 = fftshift(fft(x2,Nf));
    
    [mval mpos] = max(abs(y1).^2);
    phi_1_est = atan(imag(y1(mpos))/real(y1(mpos)));
    
    [mval mpos] = max(abs(y1).^2);
    phi_2_est = atan(imag(y2(mpos))/real(y2(mpos)));

    
    phis_fft(ii) = (phi_1_est - phi_2_est)*180/pi; 
    ii
    
    
    
end
%%
figure(1)

hist(phis_fft,50);
figure(2)
hist(phis_mdft,50,'r');

var_mdft = var(phis_mdft)
var_fft = var(phis_fft)
