function [ output_args ] = clear_socket( the_socket )
%CLEAR_SOCKET Summary of this function goes here
%   Detailed explanation goes here

if(isvalid(the_socket))
    fclose(the_socket);
    delete(the_socket);
    clear the_socket
end

end

