[FileName,PathName,FilterIndex] = uigetfile('.log')
out=horzcat(PathName,FileName);
allTable=readtable(out);
info=allTable(:,1:3);
info=info{:,:};
infoshifted=zeros(size(info));
infoshifted(2:end,:)=info(1:end-1,:);
FPS=1./(info-infoshifted);
FPS=FPS(2:end,2:2);

m=mean(FPS);
s=std(FPS);

histogram(FPS);
xlabel('FPS');
ylabel('Frame number');