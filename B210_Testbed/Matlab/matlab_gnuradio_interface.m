%% Create Properties File
close all
clear all
clc

PLOT = 0;

%Data
rx_filename1 = '../Data/data_rx1';
rx_filename2 = '../Data/data_rx2';
tx_filename = '../Data/data_tx1';

properties_file = '../Data/properties_file';

%USRP properties
Fs = 1e6; %Hz
Mclk = 24e6; %Hz
TX_gain = 15; %dB
RX_gain = 12; %dB
F_center = 700e6+ Fs; %Hz


%Signal Properties
Ts = 1/Fs;
T_sig = 0.2;
Num_samples = Fs*T_sig;
FreqStep = Fs/2;


%Connection properties
%Random port to workaround address already in use.
%TODO proper cleanup
a = 50e3;
b = 60e3;
Port = round((b-a).*rand(1) + a);
Timeout = 2.5;
Host='localhost';

%Save the file
save(properties_file)


!python /home/kampianakis/Dropbox/MainRepo/Projects/B210_Testbed/Python/file_sinks.py >> ~/Desktop/log &
pause(6)

%%
%RUN PYTHON SCRIPT FIRST

the_socket = tcpip(Host, Port);
set(the_socket,'Timeout',Timeout);
set(the_socket, 'InputBufferSize', 10000);
fopen(the_socket);

pause(0.1)

fi_rx1 = fopen(rx_filename1, 'rb');
fi_rx2 = fopen(rx_filename2, 'rb');



t = 0:Ts:T_sig-Ts;

F0 = 1e3;
x = exp(1j*2*pi*F0.*t);
% x = x + exp(1j*2*pi*2*F0.*t);
% x = x + exp(1j*2*pi*3*F0.*t);
% x = x + exp(1j*2*pi*4*F0.*t);
% x = x + exp(1j*2*pi*5*F0.*t);
% x = x./max(abs(x));


IQ = zeros(1,2*length(x));
IQ(1:2:end) = real(x);
IQ(2:2:end) = imag(x);

% 
fi_tx = fopen(tx_filename, 'wb');    %Open fifo for write binary TODO check with w only
fwrite(fi_tx,IQ,'float32');
fclose(fi_tx);

dphis = [];
damps = [];
ii = 0;
while(1)
    ii = ii+1;

    tic
    %Issue commands (In this case wait for the USRP to start)
    if(~signal_gnuradio('PR',the_socket))
        disp 'Communication error, Tryiing to revive script'
        !sudo killall python
        !python /home/kampianakis/Dropbox/MainRepo/Projects/B210_Testbed/Python/file_sinks.py >> ~/Desktop/log &
        
        disp 'Restarting..'
        pause(6)        
    end
    
    N_samples_to_read = length(x)*2 - 0.14*Fs;

    fi_rx = fopen(rx_filename1, 'rb');    
    IQ1 = fread(fi_rx ,N_samples_to_read,'float32');
    fclose(fi_rx);

    fi_rx = fopen(rx_filename2, 'rb');    
    IQ2 = fread(fi_rx,N_samples_to_read, 'float32');
    fclose(fi_rx);


    IQ_r1 = IQ1(1:2:end);
    % IQ_r1 (abs(IQ_r1) < 0.001) = 0;

    IQ_i1 = IQ1(2:2:end);
    % IQ_i1 (abs(IQ_i1) < 0.001) = 0;

    IQ_all1 = IQ_r1 + 1j*IQ_i1;

    IQ_r2 = IQ2(1:2:end);
    % IQ_r2 (abs(IQ_r2) < 0.001) = 0;

    IQ_i2 = IQ2(2:2:end);
    % IQ_i2 (abs(IQ_i2) < 0.001) = 0;
    IQ_all2 = IQ_r2 + 1j*IQ_i2;
    t_axis = 0:Ts:Ts*length(IQ_r1)-Ts;
    
    Res = 1;
    F_axis = -Fs/2 : Res : Fs/2 - 1;
    N_f = length(F_axis);
    
    y1 = fftshift(fft(IQ_all1,N_f));
    y2 = fftshift(fft(IQ_all2,N_f));

    
    if(PLOT)
        figure(1)
        subplot(1,2,1)    
        plot(t_axis,IQ_r1,t_axis, IQ_r2,'r')
        subplot(1,2,2)
        semilogy(F_axis,abs(y1).^2)
    end
    
    [mval1, mpos1] = max(abs(y1).^2);
    [mval2, mpos2] = max(abs(y2).^2);

    phi1 = atan(imag(y1(mpos1))./real(y1(mpos1)))*180/pi;
    phi2 = atan(imag(y2(mpos2))./real(y2(mpos2)))*180/pi;

    damps(ii) = 10*log10(mval1/mval2);
    dphi = phi1 - phi2;
    if(dphi < 0)
        dphi = dphi + 180;
    end
    dphis(ii) = dphi;
    timer_stop = toc
    delays(ii) = timer_stop;
    !tail -n 30 ~/Desktop/log
end


%%

x_axis = (F_center:FreqStep:(F_center +(ii-2)*FreqStep));

figure(2)
subplot(1,2,1)
plot(x_axis, smooth(dphis),x_axis,polyval(polyfit(x_axis',smooth(dphis),1),x_axis))
subplot(1,2,2)
plot(x_axis, smooth(damps),x_axis,polyval(polyfit(x_axis',smooth(damps),1),x_axis))




%% Disconnect and cleanup
fclose(the_socket);
delete(the_socket);
clear the_socket


fclose(fi_rx1);
fclose(fi_rx2);



