function [ output_args ] = kill_gnuradio( file_name)
%This function is given an input name for the script that is being executed
%and kills
[dumm1  file_name dumm2] = fileparts(file_name);
%fixme kill: No such process but it works
system(sprintf('sudo kill -n 1 $(pgrep -f %s )', file_name));
system(sprintf('sudo kill -n 1 $(pgrep -f %s )', file_name));
system(sprintf('sudo kill -n 1 $(pgrep -f %s )', file_name));
pause(1)
end

