Res = 1000;

while(1)
    Res = Res/5;
    F_axis = -Fs/2 : Res : Fs/2 - 1;
    N_f = length(F_axis);
    
    y1 = fftshift(fft(IQ_all1,N_f));
    y2 = fftshift(fft(IQ_all2,N_f));
    
    
    if(PLOT)
        figure(1)
        subplot(1,2,1)
        plot(t_axis,real(IQ_all1),t_axis, real(IQ_all2),'r')
        subplot(1,2,2)
        semilogy(F_axis,abs(y1).^2)
    end
    
    [mval1, mpos1] = max(abs(y1).^2);
    [mval2, mpos2] = max(abs(y2).^2);
    
    phi1_fft = atan(imag(y1(mpos1))./real(y1(mpos1)))*180/pi;
    phi2_fft = atan(imag(y2(mpos2))./real(y2(mpos2)))*180/pi;
    
    
    dphi = phi1_fft - phi2_fft;
    if(dphi < 0)
        dphi = dphi + 180;
    end
    pause
end
