function [ the_socket ] = setup_tcpip( the_host, the_port, the_timeout )
%SETUP_TCPIP Summary of this function goes here
%   Detailed explanation goes here
    
    the_socket = tcpip(the_host, the_port);
    set(the_socket,'Timeout',the_timeout);
    set(the_socket, 'InputBufferSize', 10000);
    fopen(the_socket);
    

end

