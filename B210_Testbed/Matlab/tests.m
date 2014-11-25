close all

res = 0.005;
new_f_axis = -Fs/2:res:Fs/2 - res;
new_nf = length(F_axis);

y_orig = fftshift(fft(x,new_nf));

y1 = fftshift(fft(IQ_all1,new_nf));

y2 = fftshift(fft(IQ_all2,new_nf));

[mval, mpos_in] = max(abs(y_orig).^2);


[mval, mpos_1] = max(abs(y1).^2);


new_f_axis(mpos1) - new_f_axis(mpos_in)
