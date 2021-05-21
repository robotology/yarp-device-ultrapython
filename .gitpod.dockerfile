FROM gitpod/workspace-full
RUN sudo apt-get update  && sudo apt-get install -y qt3d5-dev
RUN sudo git clone https://github.com/robotology/ycm && cd ~/ycm && sudo mkdir build && cd build && cmake .. && make install
