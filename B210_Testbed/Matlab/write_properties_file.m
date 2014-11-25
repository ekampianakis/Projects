close all
clear all
clc

PLOT = 1;
mdtft_on = 1;
%Data
gnuradio_script = '../Python/vector_sinks.py';
log_file = '../Data/log';
rx_filename1 = '../Data/data_rx1';
rx_filename2 = '../Data/data_rx2';
tx_filename = '../Data/data_tx1';
filetype = 'mat';
properties_file = '../Data/properties_file';

%USRP properties
Fs = 4e6; %Hz
Mclk = 6*Fs; %Hz Set this to 2*Fs to de-activate a CIC filter 
TX_gain = 15; %dB
RX_gain = 15; %dB
F_center = 700e6; %Hz
F_stop = 1150e6;
iterations = (F_stop - F_center)/(Fs/2);
Estimated_Time = (iterations*3.5)/60

%Signal Properties
Ts = 1/Fs;
T_sig = 0.6;
Num_samples = Fs*T_sig;
FreqStep = Fs/2;


%Connection properties
%Random port to workaround address already in use.
%TODO proper cleanup
a = 50e3;
b = 60e3;
Port = round((b-a).*rand(1) + a);

Timeout = 4 + T_sig;

Host='localhost';

%Save the file
save(properties_file)

