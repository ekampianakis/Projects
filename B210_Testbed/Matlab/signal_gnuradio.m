function [ output_args ] = signal_gnuradio( command,the_socket )
%SIGNAL_GNURADIO Signal the gnuradio server through tcp sockets
% Transmit data to the se rver (or a request for data from the server).
fprintf(the_socket, command);
% Pause for the communication delay, if needed.
pause(0.01)
% Receive lines of data fr om server

if(~strcmp(command,'EX'))
    DataReceived = fscanf(the_socket);
    if(strcmp(DataReceived,'MR\n') == 0)
        output_args = 1;
    elseif(length(DataReceived)<2)
        output_args = 0;
    else
        output_args = -1;
    end
else
    output_args = 1;
    
end




