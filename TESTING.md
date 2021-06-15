# 1. Testing the video stream
To test the stream I have developed the following procedure:

- execute on **iCubHead** yarpserver
- execute on **Enclustra** yarpdev
- execute on **iCubHead** the following commands

  ```
  yarpdatadumper --name /log --rxTime --txTime --type image
  yarp connect /grabber /log fast_tcp

  ```

- Analyze the data in `./log` using the Matlab scripts or manually, see above.

## 1.1. Slow movement artifact

These kind of artifacts are due to bad memory management. They should be checked by hand, looking at the singles frames.  
<img src="img/artifacts.png" width="600px">

## 1.2. Frames per second (FPS)

Fps behaviour should be analyzed with the Matlab script in `matlabscript` folder, script name `FPS_check.m`.
Once the script is executed choose the yarpdaadumper log.  
Result example:  
<img src="img/fps.png" width="600px">

## 1.3. Latency

Latency can be check manually, with a specific test. It is the delay between what is seen on YarpView and the real movement.  
<br>
It is possible also to check the latency between Enclustra and iCub-head: with the Matlab script in `matlabscript` folder, script name `Latency_check.m`.
Once the script is executed choose the yarpdaadumper log.  
<img src="img/latency.png" width="600px">
