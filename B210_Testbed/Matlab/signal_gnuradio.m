function [ output_args ] = signal_gnuradio( command,the_socket )
%SIGNAL_GNURADIO Signal the gnuradio server through tcp sockets
% Transmit data to the se rver (or a request for data from the server).
MAX_RETRIES = 5;
output_args = 0;


% Receive lines of data fr om server
retries = 0;

while(retries < MAX_RETRIES)
    fprintf(the_socket, command);
    % Pause for the communication delay, if needed.
    pause(0.01)
    
    DataReceived = fscanf(the_socket);
    
    if(~isempty(DataReceived))
        output_args = 1;
        break
    else
        retries = retries +1;
    end
end


