% Octave file for viewing the results of the
% optimization routine
close all;
pkg load ltfat;
% Naive (initial) guess at inverter parameters
naive=load("naive.txt");
% No control solution
ref=load("ref.txt");
% Output from the optimizer
best=load("soln.txt");
plotfft(fft(ref(:,2)),3000,'posfreq');
hold on;
%plotfft(fft(naive(:,2)),3000,'posfreq');
plotfft(fft(best(:,2)),3000,'posfreq');
