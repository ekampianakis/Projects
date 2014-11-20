function [ output_args ] = restart_gnuradio( file_name,log_file )
%RESTART_GNURADIO Summary of this function goes here
%   Detailed explanation goes here
disp('Killing Gnuradio') 
kill_gnuradio(file_name);
disp('Starting Gnurdadio') 

start_gnuradio(file_name,log_file);
disp('Started Gnurdadio') 

pause(6);
disp('Started Gnurdadio') 

end

