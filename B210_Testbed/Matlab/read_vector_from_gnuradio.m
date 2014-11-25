function [ IQ ] = read_vector_from_gnuradio( rx_filename,N_samples,type )
%READ_VECTOR_FROM_GNURADIO Summary of this function goes here
%   Detailed explanation goes here

    if(strcmp(type,'bin'))
        fi_rx = fopen(rx_filename, 'rb');    
        IQ = fread(fi_rx,N_samples*2 , 'float32');
        if(mod(length(IQ),2) == 0)
            IQ =  IQ(1:2:end) + 1j*IQ(2:2:end);
        else
            IQ =  IQ(1:2:end-1) + 1j*IQ(2:2:end);
        end

        fclose(fi_rx);
    elseif(strcmp(type,'mat'))
        load(rx_filename)
        IQ = vector(1:N_samples);
    end
       


end

