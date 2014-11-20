function [ output_args ] = print_log( line_num, log_file )
    
    
    system(sprintf('tail -n %d %s',line_num,log_file));

end

