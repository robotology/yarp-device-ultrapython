% Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
% All rights reserved.
%
% This software may be modified and distributed under the terms of the
% BSD-3-Clause license. See the accompanying LICENSE file for details.
%


%read file from yarpdatadumber format see above
%
%yarpdatadumper --name /log --rxTime --txTime --type image
%yarp connect /grabber /log fast_tcp
%

removedSample=10;%Removed frames from tail and queue before start

[FileName,PathName,FilterIndex] = uigetfile('.log')
out=horzcat(PathName,FileName);
allTable=readtable(out);

%calculate
info=allTable(removedSample:end-removedSample,2:3);%remove first and last frames
info=info{:,:};
latency=(info(:,2)-info(:,1))*1000

%mean
m=mean(latency);
s=std(latency);

%show all plots
tiledlayout(1,1)
nexttile
histogram(latency);
xlabel('latency msec');
ylabel('Frame number');
title('Frame distribution')
grid on
