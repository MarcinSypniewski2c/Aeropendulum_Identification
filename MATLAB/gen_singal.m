%%% File info 
%
% ************************************************************************
%
%  @file     gen_signal.m
%  @author   AW     Adrian.Wojcik@put.poznan.pl
%  @version  1.0
%  @date     13-May-2022 11:09
%  @brief    Generate reference speed signal for identification
%
% ************************************************************************
%
Ts = 50e-3;
S = [];
for n = 1 : 10
    S1 = sort([0 1 rand(1,4)], 'ascend');
    S2 = sort([1 -1 rand(1,4) -rand(1,4)], 'descend');
    S3 = sort([-1 0 -rand(1,4)], 'ascend');
    S = [S S1 S2 S3];
end

N = 100;
s = N*length(S);

for i = 1 : length(S)
    s((1:N) + (i-1)*N) = S(i);
end

speed_ref = timeseries(s', (0 : (length(s)-1))*Ts);
SPEED_MAX = 3000;
sim("gen_signal.slx");
writematrix(round(out.speed_ref_csv.data)', "speed_ref.csv");