FROM icubteamcode/ubuntu2004mesa:master_sources

LABEL org.opencontainers.image.title="YARP UltraPython Device Docker Image"
LABEL org.opencontainers.image.description="Infrastructure to deal with the UltraPython YARP device"
LABEL org.opencontainers.image.source="https://github.com/robotology/yarp-device-ultrapython"
LABEL org.opencontainers.image.authors="Luca Tricerri <luca.tricerri@iit.it>"

ENV DEBIAN_FRONTEND=noninteractive

# Select options
ARG ROBOTOLOGY_SUPERBUILD_RELEASE
ARG ULTRAPYTHON_RELEASE
ARG BUILD_TYPE=Release
ARG PROJECTS_DIR=/projects
ARG CMAKE_GENERATOR="Unix Makefiles"
ARG CMAKE_EXTRA_OPTIONS=-j4
ARG INSTALL_DIR=/usr/local

RUN mkdir -p ${PROJECTS_DIR}

# Install initial dependencies
RUN mkdir -p /etc/bash_completion.d/ &&\
    apt update &&\
    apt install -y \
    # misc
    git \
    vim \
    htop \
    net-tools \
    psmisc \
    wget \
    dos2unix \
    # ultrapython
    libudev-dev \
    screen

# Set up git (required by superbuild)
RUN git config --global user.name "GitHub Actions" && \
    git config --global user.email "actions@github.com"

# Install dependencies through robotology-superbuild
RUN cd ${PROJECTS_DIR} &&\
    git clone https://github.com/robotology/robotology-superbuild.git --branch ${ROBOTOLOGY_SUPERBUILD_RELEASE} &&\
    robotology-superbuild/scripts/install_apt_dependencies.sh

RUN cd ${PROJECTS_DIR}/robotology-superbuild &&\
    mkdir build && cd build &&\
    cmake .. \
    -G "${CMAKE_GENERATOR}" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DNON_INTERACTIVE_BUILD:BOOL=ON \
    -DROBOTOLOGY_ENABLE_CORE:BOOL=ON \
    -DYCM_USE_DEPRECATED:BOOL=OFF \
    -DROBOTOLOGY_USES_GAZEBO=OFF \
    -DYCM_EP_INSTALL_DIR=${INSTALL_DIR} &&\
    cmake --build . -- ${CMAKE_EXTRA_OPTIONS} &&\
    cd ../../ && rm -Rf robotology-superbuild

# Setup entrypoint
ARG ROBOTOLOGY_INITIALIZATION_FILE=/usr/local/bin/setup_robotology_tdd.sh
RUN echo "source ${INSTALL_DIR}/share/robotology-superbuild/setup.sh" >> ${ROBOTOLOGY_INITIALIZATION_FILE}

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN  dos2unix /usr/local/bin/entrypoint.sh &&\
     chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT [ "/usr/local/bin/entrypoint.sh" ]

# The bashrc is read only when opening an interactive shell.
# Thus, let other projects find packages contained in the superbuild.
ENV CMAKE_PREFIX_PATH=${INSTALL_DIR}

# ultrapython
RUN cd ${PROJECTS_DIR} &&\
    git clone https://github.com/robotology/yarp-device-ultrapython.git --branch ${ULTRAPYTHON_RELEASE} &&\
    cd yarp-device-ultrapython &&\
    mkdir build && cd build &&\
    cmake .. \
    -G "$CMAKE_GENERATOR" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCOMPILE_WITHUI_ULTRAPYTHON=ON \
    -DNON_INTERACTIVE_BUILD:BOOL=ON &&\
    cmake --build . -- ${CMAKE_EXTRA_OPTIONS}
ENV PATH=${PATH}:${PROJECTS_DIR}/yarp-device-ultrapython/clientScript:${PROJECTS_DIR}/yarp-device-ultrapython/build/bin

# Clean up git configuration
RUN git config --global --unset-all user.name && \
    git config --global --unset-all user.email

EXPOSE 10000/tcp 10000/udp

# Some QT-Apps don't show controls without this
ENV QT_X11_NO_MITSHM 1
ENV YARP_COLORED_OUTPUT 1

ARG metadata="none"
ENV img_metadata=${metadata}
RUN echo 'echo 'This image has UltraPython release=${ULTRAPYTHON_RELEASE} and had been built with robotology-superbuild release=${ROBOTOLOGY_SUPERBUILD_RELEASE}. Metadata=${metadata} ' '  >> ${ROBOTOLOGY_INITIALIZATION_FILE}

# Clean up unnecessary installation products
RUN rm -Rf /var/lib/apt/lists/*

CMD ["bash"]
