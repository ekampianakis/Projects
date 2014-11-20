function [ output_args ] = start_gnuradio( file_name,log_file )
%START_GNURADIO Summary of this function goes here

%TODO add log filename too
system(sprintf('python %s  > %s &', file_name,log_file));
system(sprintf('tail -n 100 %s ', log_file));


pause(8)
end

