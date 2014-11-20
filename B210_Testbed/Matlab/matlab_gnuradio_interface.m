%% Create Properties File
close all
clear all
clc

write_properties_file
restart_gnuradio(gnuradio_script,log_file)


the_socket = setup_tcpip(Host,Port, Timeout);
pause(0.1)


%%
t = 0:Ts:T_sig-Ts;


F0 = 1e3;

x = exp(1j*2*pi*F0.*t);

% x = exp(1j*2*pi*F0.*t);
% x = x + exp(1j*2*pi*2*F0.*t);
% x = x + exp(1j*2*pi*3*F0.*t);
% x = x + exp(1j*2*pi*4*F0.*t);
% x = x + exp(1j*2*pi*5*F0.*t);
% x = x./max(abs(x));

write_vector_to_gnuradio(tx_filename,x)


dphis_fft = [];
damps = [];
Freq_now = F_center;
%%
for(ii = 1:900)
    tic
    
    ii    
    Freq_now = Freq_now + FreqStep
        
    %Issue commands (In this case wait for the USRP to start)
    while(~signal_gnuradio(sprintf('PR.%f',Freq_now),the_socket))
        disp 'Gnuradio not responding, restarting..'   
        kill_gnuradio(gnuradio_script) 
        pause(2)
        clear_socket(the_socket)                
        
        %change the port (TODO fix that)
        Port = Port+1;
        save('../Data/properties_file.mat','Port','-append')
        
        start_gnuradio(gnuradio_script,log_file);        
        pause(6)
        
        the_socket =  setup_tcpip(Host,Port, Timeout);        
        pause(0.1)        
    end
    
    N_samples_to_read = length(x)*2;
    
    IQ_all1 = read_vector_from_gnuradio(rx_filename1,N_samples_to_read);
    IQ_all2 = read_vector_from_gnuradio(rx_filename2,N_samples_to_read);
    
    t_axis = 0:Ts:Ts*length(IQ_all1)-Ts;
    
    
    Res = 1;
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
    
    m_settings = struct('fs',Fs,'f0',F0);
    [dphi_estimator(ii) d_error] = mdtft(real(IQ_all1),real(IQ_all2),m_settings);
    dphi_estimator(ii) = dphi_estimator(ii)*180/pi; 
    
    damps(ii) = 10*log10(mval1/mval2);
    dphi = phi1_fft - phi2_fft;
    if(dphi < 0)
        dphi = dphi + 180;
    end
    dphis_fft(ii) = dphi;
    timer_stop = toc
    delays(ii) = timer_stop;
    
    print_log(60,log_file)
end


%%

x_axis = (F_center:FreqStep:(F_center +(ii-2)*FreqStep));

figure(2)
subplot(1,3,1)
plot(x_axis, (dphis_fft),x_axis,polyval(polyfit(x_axis,(dphis_fft),1),x_axis))
title('Phase est with fft vs freq')
axis([min(x_axis) max(x_axis) 0  180])
subplot(1,3,2)
plot(x_axis, (dphi_estimator),x_axis,polyval(polyfit(x_axis,(dphi_estimator),1),x_axis))
axis([min(x_axis) max(x_axis) 0  180])

title('Phase est with mdtft vs freq')
subplot(1,3,3)
plot(x_axis, (damps),x_axis,polyval(polyfit(x_axis,(damps),1),x_axis))
title('Amp diff(dB) vs freq')
axis([min(x_axis) max(x_axis) min(damps) max(damps)])



%% Disconnect and cleanup

%%%SOS Define a save_filename first
kill_gnuradio(gnuradio_script)
% clear_and_save
clear_socket(the_socket)


