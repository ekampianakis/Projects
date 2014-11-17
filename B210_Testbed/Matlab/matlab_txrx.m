clear all
clc

%V4 re-write everything
%V5 added simple fft phase calc and
%V6 carrier freqs in a vector, various filters implemented, phase
%estimation with mdft


%--------------- Script Parameters ---------------
TX = 1;         %Enable TX
RX = 1 ;        %enable RX
PLOT = 0;       %Enable plotting


%--------------- Radio Parameters ---------------
Fs = 1e6    %Sampling rate in Hz t
Ts = 1/Fs;  %Sampling period


%--------------- Carrier Parameters ---------------


% F_carrier_start = 90e3;
% F_carrier_stop = 360e3;
%
% F_spacing = 90e3;   %Spacing between carriers


% F_carrier_axis = F_carrier_start : F_spacing : F_carrier_stop
% F_carrier_axis = [100e3 200e3 300e3 400e3]
F_carrier_axis = [20e3]

N_carriers = length(F_carrier_axis);    %umber of carriers


amps_carriers = ones(1,N_carriers);     %create the vector of amps for each carrier
phi_carriers = zeros(1,N_carriers);     %create the vector of phases for each carrier


%--------------- Estimation Parameters ---------------
phi_tag_est = zeros(1,N_carriers);
f_tag_est = zeros(1,N_carriers);
amp_tag_est = zeros(1,N_carriers);


%--------------- TX Parameters ---------------
if(TX)
    T_tx = 0.3;           %Total TX time
    t_tx = 0:Ts:T_tx-Ts;   %TX Time axis
    
    fi_tx = fopen('/Users/kampianakis/Desktop/SGCC/Demos/B211StDemo/usrp_tx', 'wb');    %Open fifo for write binary TODO check with w only
    x_tx = zeros(1,length(t_tx)*2);                                                     %Init vector that saves RX data
    
    x_quad = zeros(1,length(t_tx));
    
end


%--------------- RX Parameters ---------------
if(RX)
    DROP_RATE = 1;          %Drop packet rate on RX
    T_rx = 0.3;             %Total RX time
    t_rx = 0:Ts:T_rx-Ts;    %RX Time axis
    PACKET_DETECT = 0;      %Detect packet or not
    
    FILTERING_FILT_FILT = 0;%enable bpf filtfilt filtering around tag
    FILTERING_FFT =0;      %Enable fft filtering same as msc thesis
    SHIFTING = 0;           %enable carrier shifting
    
    FFT_FILTER_BW = 1e3;
    
    
    T_rx_min = 0.079;       %packet start (depends on T_rx, check before you change)
    T_rx_max = T_rx_min+T_tx; %0.145;       %packet stop  (same here)
    fi_rx = fopen('/Users/kampianakis/Desktop/SGCC/Demos/B211StDemo/usrp_rx', 'rb');    %Open fifo for write binary TODO check with w only
    x_rx = zeros(1,length(t_rx)*2);                                                     %Init vector that saves RX data
    N_samples_rx = length(x_rx);                                                        %Number of samples that will be read from rx_fifo.
    packet_counter = 0;                                                                 %Counter that is used to drop packets for sped up processing
end

%--------------- FFT Parameters ---------------
Resolution = 1;                                 %Resolution of fourier in Hz
F_axis = -Fs/2:Resolution:Fs/2-Resolution;      %Frequency axis
N_f = length(F_axis);                           %Number of bins


%--------------- Tag Parameters ---------------
F_tag = 5e3;
%bpf = designfilt('bandpassfir', 'StopbandFrequency1', 1e3, 'PassbandFrequency1', 3e3, 'PassbandFrequency2', 7e3, 'StopbandFrequency2', 9e3, 'StopbandAttenuation1', 60, 'PassbandRipple', 1, 'StopbandAttenuation2', 60, 'SampleRate', 1000000);
% tag_filt = designfilt('bandpassfir', 'StopbandFrequency1', 1e3, 'PassbandFrequency1', F_tag, 'PassbandFrequency2', 11*F_tag, 'StopbandFrequency2', 13*F_tag, 'StopbandAttenuation1', 20, 'PassbandRipple', 1, 'StopbandAttenuation2', 20, 'SampleRate', 1000000);
% tag_filt = designfilt('bandpassfir', 'StopbandFrequency1', 1e3, 'PassbandFrequency1', F_tag, 'PassbandFrequency2', 12*F_tag, 'StopbandFrequency2', 14*F_tag, 'StopbandAttenuation1', 20, 'PassbandRipple', 1, 'StopbandAttenuation2', 20, 'SampleRate', 1000000);
tag_filt = designfilt('lowpassfir', 'PassbandFrequency', 50e3, 'StopbandFrequency', 70e3, 'PassbandRipple', 1, 'StopbandAttenuation', 20, 'SampleRate', 1000000);
% load bpf2



k = 1;

while(1)
    
    %-----------------------TX-------------------------------
    if(TX)
        
        
        for(ii = 1:N_carriers)
            x_quad = x_quad + amps_carriers(ii).*exp(1j*2*pi*F_carrier_axis(ii)*t_tx + phi_carriers(ii));      %generate multiple carriers
        end
        
        x_quad = x_quad./max(abs(x_quad));               %normalize amplitude
        
        
        
        
        %         x_tx(2:2:end) = fliplr(real(x_quad));               %interleave I flip vector (test)
        %         x_tx(1:2:end) = fliplr(imag(x_quad));               %interleave Q
        
        x_tx(2:2:end) = imag(x_quad);
        x_tx(1:2:end) = real(x_quad);
        
        count = fwrite(fi_tx, x_tx, 'float32');     %write to fifo
        
        if(count ~= length(x_tx))                   %if something goes wrong quit
            disp('Write error');
            return;
        end
        
        
    end %TX
    
    %-----------------------RX-------------------------------
    if(RX)
        packet_counter = packet_counter + 1;                        %inc pack counter
        
        
        if(mod(packet_counter,DROP_RATE) == 0)                      %drop keep 1 out of DROP_RATE
            packet_counter = 0;                                     %reset
            
            x_rx = fread(fi_rx, N_samples_rx, 'float32');           %read fifo
            x_rx = x_rx(1:2:end) + 1j*x_rx(2:2:end);                %deinterleave
            
            if(PACKET_DETECT)
                
                t_rx_packet = t_rx(t_rx > T_rx_min & t_rx < T_rx_max);        %quick and dirty packet detection
                x_rx_packet = x_rx(find(t_rx > T_rx_min & t_rx < T_rx_max));
                
            else
                
                t_rx_packet = t_rx;        %do nothing
                x_rx_packet = x_rx;
                
            end
            if(SHIFTING)                                        %Enable shifting of carriers to baseband
                x_rx_orig = x_rx_packet;                        %Copy packet to a temp vector
                for(ii = 1:N_carriers)
                    
                    x_rx_packet = x_rx_orig.*exp(1j*2*pi*(F_carrier_axis(ii))*t_rx_packet)'; %Shift to bring carrier at baseband
                    
                    
                    if(FILTERING_FILT_FILT)
                        x_rx_packet = filtfilt(tag_filt,x_rx_packet);       %Baseband filter around tag
                        
                    end
                    
                    x_fft_rx = fftshift(fft(x_rx_packet,N_f)).*Ts;             %fftis
%                     x_fft_rx(F_axis<= F_tag -1e3) = 0;
                    
                    if(FILTERING_FFT)                                       %instead of filtfilt(slow) use fft bandpass filter
                        
                        LEFT = F_tag - FFT_FILTER_BW;                       %Cut left and right of tag
                        RIGHT = F_tag + FFT_FILTER_BW;
                        
                        x_fft_rx(F_axis < -RIGHT) = 0;                      %zero out all that doesnt contain the tag's subcarrier
                        x_fft_rx(F_axis > RIGHT) = 0;
                        x_fft_rx(abs(F_axis) < LEFT) = 0;
                        
                    end
                    
                    x_rx_periodog = abs(x_fft_rx).^2;                      %periodogram calculation
                    
                    [mval mpos] = max(x_rx_periodog);      %Find the POSITIVE freq. component that with the largest amplitude
                    
                    f_tag_est(ii) = F_axis(mpos);                                   %Find tag's frequency ths should be F_tag
                    %                                         phi_tag_est(ii) = atan2(imag(x_fft_rx(mpos)),real(x_fft_rx(mpos))); %Find tag's phase angle
                    
                    phase_est_settings = struct('fs',Fs,'f0',F_tag);
                    [phi_tag_est(ii), dumm] = mdtft(imag(x_rx_packet),real(x_rx_packet),phase_est_settings);
                    amp_tag_est(ii) = max(x_rx_periodog);
                    
                    if PLOT
                        plot_time_freq(t_rx_packet,x_rx_packet,F_axis,x_rx_periodog)
                    end %PLOT
                    
                end %carrier run
                
            else%NOT SHIFTING
                
                for(ii = 1:N_carriers)
                    
                    Fith_carrier = F_carrier_axis(ii);                 %Find the ith carrier
                    
                    LEFT = F_tag - FFT_FILTER_BW + Fith_carrier ;       %Cut left and right of tag
                    RIGHT = F_tag + FFT_FILTER_BW +Fith_carrier ;
                    
                    
                    x_fft_rx = fftshift(fft(x_rx_packet,N_f)).*Ts;          %fftis
                    
                    if(FILTERING_FFT)
                        x_fft_rx(F_axis > RIGHT) = 0;
                        x_fft_rx(F_axis < LEFT) = 0;            %TODO! Need to add for the negative tag subcarrier
                    end
                    
                    
                    x_rx_periodog = abs(x_fft_rx).^2;       %periodogram calculation
                    
                    [ amp_tag_est(ii) mpos] = max(x_rx_periodog);      %Find the POSITIVE freq. component that with the largest amplitude
                    
                    f_tag_est(ii) = F_axis(mpos);                                   %Find tag's frequency ths should be F_tag
                    %                     phi_tag_est(ii) = atan2(imag(x_fft_rx(mpos)),real(x_fft_rx(mpos))); %Find tag's phase angle
                    
                    phase_est_settings = struct('fs',Fs,'f0',F_tag);
                    [phi_tag_est(ii), dumm] = mdtft(imag(x_rx_packet),real(x_rx_packet),phase_est_settings);
                    
                    if PLOT
                        plot_time_freq(t_rx_packet,x_rx_packet,F_axis,x_rx_periodog)
                    end %PLOT
                    
                    
                end
                
            end %SHIFTING
            
            
            f_tag_est
            
            phi_tag_est = phi_tag_est*180/pi
            
            phi_tag_ests{k} =  phi_tag_est;
            
            k = k+1;
            
            %             phi_tag_est = phi_tag_est*180/pi
            amp_tag_est

figure(2);
tmin = 0.095;
tmax = 0.0955;
inds = find(t_rx_packet < tmax & t_rx_packet > tmin);
gain = 300;
clf(2)






hold on

plot(t_rx_packet(inds),gain*real(x_rx_packet(inds)),t_tx(inds),gain*imag(x_rx_packet(inds)))

plot(t_tx(inds),real(x_quad(inds)),'r',t_tx(inds),imag(x_quad(inds)),'m')
hold off
            
            
            
            if(PLOT)
                figure(2)
                plot(F_carrier_axis,amp_tag_est);
                
                
            end %PLOT
            
        end; %mod
        
    end %rx
    
end %while


%%




