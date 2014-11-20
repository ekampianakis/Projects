function [ IQ ] = read_vector_from_gnuradio( rx_filename,N_samples )
%READ_VECTOR_FROM_GNURADIO Summary of this function goes here
%   Detailed explanation goes here

    fi_rx = fopen(rx_filename, 'rb');    
    IQ = fread(fi_rx,N_samples , 'float32');
    if(mod(length(IQ),2) == 0)
        IQ =  IQ(1:2:end) + 1j*IQ(2:2:end);
    else
        IQ =  IQ(1:2:end-1) + 1j*IQ(2:2:end);
    end
    
    fclose(fi_rx);

end

