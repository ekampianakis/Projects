function [ output_args ] = signal_gnuradio( command,the_socket )
%SIGNAL_GNURADIO Signal the gnuradio server through tcp sockets
% Transmit data to the se rver (or a request for data from the server).
MAX_RETRIES = 5;
output_args = 0;


% Receive lines of data fr om server
retries = 0;

if(isvalid(the_socket))
while(retries < MAX_RETRIES)
    fprintf(the_socket, command);
    % Pause for the communication delay, if needed.
    pause(0.01)
    
    if(strcmp(the_socket.Status,'open')==1)
        DataReceived = fscanf(the_socket);
    else
        output_args = 0;
        return
    end
    if(~isempty(DataReceived))
        output_args = 1;
        break
    else
        retries = retries +1;
    end
end
else
    output_args = 0;
end

