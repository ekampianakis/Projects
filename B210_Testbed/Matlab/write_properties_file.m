close all
clear all
clc

PLOT = 0;

%Data
gnuradio_script = '../Python/file_sinks.py';
log_file = '../Data/log';
rx_filename1 = '../Data/data_rx1';
rx_filename2 = '../Data/data_rx2';
tx_filename = '../Data/data_tx1';

properties_file = '../Data/properties_file';

%USRP properties
Fs = 2.5e6; %Hz
Mclk = 2*Fs; %Hz Set this to 2*Fs to de-activate a CIC filter 
TX_gain = 15; %dB
RX_gain = 15; %dB
F_center = 700e6; %Hz


%Signal Properties
Ts = 1/Fs;
T_sig = 0.3;
Num_samples = Fs*T_sig;
FreqStep = Fs/2;


%Connection properties
%Random port to workaround address already in use.
%TODO proper cleanup
a = 50e3;
b = 60e3;
Port = round((b-a).*rand(1) + a);

if(Mclk <=10e6)
    Timeout = 4 + T_sig;
else
    Timeout = 2.5 + T_sig;
end
Host='localhost';

%Save the file
save(properties_file)

