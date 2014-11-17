%% Create Properties File
close all
clear all
clc
%Data
rx_filename1 = '../Data/data_rx1';
rx_filename2 = '../Data/data_rx2';
tx_filename = '../Data/data_tx1';

properties_file = '/Users/kampianakis/Documents/Everything/Dropbox/MainRepo/Projects/B210_Testbed/Data/properties_file';

%USRP properties
Fs = 1e6; %Hz
Mclk = 10e6; %Hz
TX_gain = 10; %dB
RX_gain = 10; %dB
F_center = 915e6; %Hz


%Connection properties
%Random port to workaround address already in use.
%TODO proper cleanup
a = 50e3;
b = 60e3;
Port = round((b-a).*rand(1) + a);

Host='localhost';

%Save the file
save(properties_file)




%%
%RUN PYTHON SCRIPT FIRST

the_socket = tcpip(Host, Port);
set(the_socket,'Timeout',15);
set(the_socket, 'InputBufferSize', 10000);
fopen(the_socket);



% fi_tx = fopen(tx_filename,'wb');
pause(0.2)

%Issue commands (In this case wait for the USRP to start)
if(~signal_gnuradio('PR',the_socket))
    disp 'Communication error'
    return
end
fi_tx = fopen(tx_filename, 'wb');
fi_rx1 = fopen(rx_filename1, 'rb');
fi_rx2 = fopen(rx_filename2, 'rb');


%%
while(1)
    
    Ts = 1/Fs;
    T_sig = 0.2;
    t = 0:Ts:T_sig-Ts;
    
    F0 = 2e3;
    x = exp(1j*2*pi*F0.*t);
    
    IQ = zeros(1,2*length(x));
    IQ(1:2:end) = real(x);
    IQ(2:2:end) = imag(x);
    
    fseek(fi_tx,0,'bof');
    fwrite(fi_tx,IQ,'float32');
    
    if(~signal_gnuradio('PR',the_socket))
        disp 'Communication error'
        return
    end 
    
    pause(0.5)
    
    if(~signal_gnuradio('PR',the_socket))
        disp 'Communication error'
        return
    end
   
    
    fseek(fi_rx1,0,'bof');
    IQ1 = fread(fi_rx1 ,length(x)*2,'float32');
    fseek(fi_rx2,0,'bof');
    IQ2 = fread(fi_rx2,length(x)*2, 'float32');
    
    IQ_all1 = IQ1(1:2:end) + 1j*IQ1(2:2:end);
    IQ_all2 = IQ2(1:2:end) + 1j*IQ2(2:2:end);
    
    
    figure(1)
    
    plot(1:length(IQ_all1),real(IQ_all1),'b',1:length(IQ_all2),real(IQ_all2),'r')
    
    
    
    Res = 1;
    F_axis = -Fs/2 : Res : Fs/2 - 1;
    N_f = length(F_axis);

    y1 = fftshift(fft(IQ_all1,N_f));
    y2 = fftshift(fft(IQ_all2,N_f));

    figure(2)
    semilogy(F_axis,abs(y1).^2)


    pause;
 

end

%% Disconnect and cleanup
fclose(the_socket);
delete(the_socket);
clear the_socket
%%
fclose(fi_tx);
fclose(fi_rx1);
fclose(fi_rx2);



