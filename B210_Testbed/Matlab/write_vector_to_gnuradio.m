function [ output_args ] = write_vector_to_gnuradio( tx_filename,x )
%WRITE_VECTOR_TO_GNURADIO Summary of this function goes here
%   Detailed explanation goes here

% 

IQ = zeros(1,2*length(x));
IQ(1:2:end) = real(x);
IQ(2:2:end) = imag(x);

fi_tx = fopen(tx_filename, 'wb');    %Open fifo for write binary TODO check with w only
fwrite(fi_tx,IQ,'float32');
fclose(fi_tx);
end

