%read file
[FileName,PathName,FilterIndex] = uigetfile('.log')
out=horzcat(PathName,FileName);
allTable=readtable(out);

%calculate
info=allTable(:,1:3);
info=info{:,:};
infoshifted=zeros(size(info));
infoshifted(2:end,:)=info(1:end-1,:);
FPS=1./(info-infoshifted);
FPS=FPS(2:end,2:2);

%mean
m=mean(FPS);
s=std(FPS);

%show
histogram(FPS);
xlabel('FPS');
ylabel('Frame number');