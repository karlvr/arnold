FROM cpcsdk/crossdev
MAINTAINER giot.romain@gmail.com

USER root
RUN apt-get update && \
    apt-get install -qy \
        fossil \
        pasmo \
        libsdl-image1.2 libsdl1.2-dev libsdl1.2debian \
        libwxbase3.0-0v5 libwxbase3.0-dev libwxgtk3.0-0v5 libwxgtk3.0-dev wx3.0-headers
        
# Add 2cdt
# TODO Backport that in cpcsdk/crossdev
WORKDIR /tmp
RUN wget http://cpctech.cpc-live.com/download/2cdt.zip -O 2cdt.zip && \
    unzip 2cdt.zip && \
    cd 2cdt && \
    make 2cdt && \
    cp 2cdt /usr/local/bin
        
        
# Add playtzx
WORKDIR /src
RUN git clone --depth 1    https://github.com/ralferoo/cpctools.git ralferoo && \
    cd ralferoo/playtzx-0.12b && \
    ./configure && \
    make install
    
USER arnold    
RUN mkdir -p /home/arnold/arnoldemu
WORKDIR /home/arnold/arnoldemu

