
pathName = uigetdir();
cd(pathName);

firstImage = imread('1.ppm');
sumImage = double(firstImage); 

%count files
d = dir(['./', '/*.ppm']);
imgnumber=length(d)-1;

%mean
for i=2:imgnumber 
  currentImage = imread([num2str(i),'.ppm']);
  sumImage = sumImage + double(currentImage);
end;
meanImage = sumImage / imgnumber;

%show
figure;
imshow(uint8(firstImage));
title("First image");
figure;
imshow(uint8(meanImage));
title("Mean");