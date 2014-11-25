function [ output_args ] = write_vector_to_gnuradio( tx_filename,TX_vector,type )
%WRITE_VECTOR_TO_GNURADIO Summary of this function goes here
%   Detailed explanation goes here

%
    if(strcmp(type,'mat'))
        if(exist(tx_filename,'file'))
            save(tx_filename,'TX_vector','-append')
        else
            save(tx_filename,'TX_vector')
        end
    elseif(strcmp(filetype,'bin'))
        IQ = zeros(1,2*length(TX_vector));
        IQ(1:2:end) = real(TX_vector);
        IQ(2:2:end) = imag(TX_vector);

        fi_tx = fopen(tx_filename, 'wb');    %Open fifo for write binary TODO check with w only
        fwrite(fi_tx,IQ,'float32');
        fclose(fi_tx);
    end
    
end

