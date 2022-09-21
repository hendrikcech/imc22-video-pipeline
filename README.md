# Adaptive Video Delivery Pipeline

This project implements a video delivery pipeline that transfers live video from a client to a server using RTP.
The code is based on Mathis Engelbart's project [rtp-over-quic](https://github.com/mengelbart/rtp-over-quic) (“roq”)
and was adapted to support our work on remote piloting of aerial vehicles that was published at the ACM Internet Measurement Conference (ACM IMC) 2022: [Analyzing Real-time Video Delivery over Cellular Networks for Remote Piloting Aerial Vehicles](https://doi.org/10.1145/3517745.3561465).

Related code is available at [hendrikcech/imc22-remote-piloting](https://github.com/hendrikcech/imc22-remote-piloting).
The video that we used during our tests can be obtained at [mediaTUM](https://mediatum.ub.tum.de/1687221) (directory `video/train_30.mp4`).

The program supports the two live media congestion control algorithms
[GCC](https://datatracker.ietf.org/doc/draft-ietf-rmcat-gcc/02/)
and
[SCReAM](https://datatracker.ietf.org/doc/html/rfc8298).
The GCC implementation is taken from [pion/interceptor](https://github.com/pion/interceptor), while the SCReAM implementation is based on commit 75cd6fe of [EricssconResearch/scream](https://github.com/EricssonResearch/scream/tree/75cd6fe9c935a55da21228fd882cfab397e29265).

This README will show you how to [build](#build-roq) and [use](#usage) the video pipeline ROQ (short for “rtp-over-quic”).

## Build the application
This project requires GStreamer, the [gst-timecode](https://github.com/hendrikcech/gst-timecode) Gstreamer element, and go 1.17.
You can use `setup_environment.sh` to automatically fetch and compile these dependencies.
The script will leave you with a shell in which you can use the video delivery pipeline.

If you don't want to use `setup_environment.sh`, please follow the steps below.

### 1. Compile GStreamer
The GStreamer clocksync element is required which is only available since GStreamer 1.20.
Either install that version from a package repository or compile GStreamer from source (https://gitlab.freedesktop.org/gstreamer/gstreamer/).
The following commands will compile a version of GStreamer that has only the necessary features available.

``` sh
git clone -b 1.20.1 https://gitlab.freedesktop.org/gstreamer/gstreamer.git

# install the build systems
pip3 install ninja meson

```

#### Sender-Side Compilation
``` sh
meson \
    -Dbuildtype=release \
    -Dauto_features=disabled \
    -Dbase=enabled \
    -Dgood=enabled \
    -Dbad=enabled \
    -Dgst-plugins-base:videotestsrc=enabled \
    -Dgst-plugins-base:tcp=enabled \
    -Dgst-plugins-base:app=enabled \
    -Dgst-plugins-good:rtp=enabled \
    -Dgst-plugins-good:udp=enabled \
    -Dgst-plugins-base:playback=enabled \
    -Dgst-plugins-bad:videoparsers=enabled \
    -Dgst-plugins-base:videoconvert=enabled \
    -Dgst-plugins-good:avi=enabled \
    -Dgst-plugins-good:isomp4=enabled \
    -Dlibav=enabled \
    -Dgpl=enabled \
    -Dugly=enabled \
    -Dgst-plugins-ugly:x264=enabled \
    builddir

# To install gstreamer system-wide
sudo meson install -C builddir

# Or just compile GStreamer and switch to the environment
ninja -C builddir
ninja -C builddir devenv
```

#### Receiver-Side Compilation
```sh
meson \
    -Dauto_features=disabled \
    -Dbuildtype=release \
    -Dbase=enabled \
    -Dbad=enabled \
    -Dgood=enabled \
    -Dgst-plugins-base:tcp=enabled \
    -Dgst-plugins-base:playback=enabled \
    -Dgst-plugins-base:videoconvert=enabled \
    -Dgst-plugins-base:app=enabled \
    -Dgst-plugins-bad:debugutils=enabled \
    -Dgst-plugins-bad:videoparsers=enabled \
    -Dgst-plugins-good:rtp=enabled \
    -Dgst-plugins-good:rtpmanager=enabled \
    -Dgst-plugins-good:udp=enabled \
    -Dgst-plugins-good:avi=enabled \
    -Dgst-plugins-good:videobox=enabled \
    -Dgst-plugins-good:videocrop=enabled \
    -Dlibav=enabled \
    -Dgstreamer:coretracers=enabled \
    builddir

sudo meson install -C builddir
```

### 2. Build gst-timecode
Download, build, and install the `timecodeoverlay` and `timecodeparse` GStreamer elements as described in [hendrikcech/gst-timecode](https://github.com/hendrikcech/gst-timecode).

### 3. Setup go and build the application
Due to the dependency on quic-go, the project has to be built with go 1.17 which can be fetched from [golang.org](https://go.dev/dl/#go1.17.13).

``` sh
wget https://go.dev/dl/go1.17.13.linux-amd64.tar.gz
tar -xzf go1.17.13.linux-amd64.tar.gz

go/bin/go build -o roq
```

## Usage
The commands below show how to start sender and receiver on the same machine with three different bitrate adaption methods: GCC, SCReAM, and constant bitrate (static).

### Receiver
The value of `--fps-dump` can either be `stdout` to log to stdout or the path to a file.
```sh
# GCC
./roq receive -a :4242 --sink fpsdisplaysink --fps-dump stdout --save rcvr.avi --transport udp --twcc

# SCReAM
./roq receive -a :4242 --sink fpsdisplaysink --fps-dump stdout --save rcvr.avi --transport udp --rfc8888

# static
./roq receive -a :4242 --sink fpsdisplaysink --fps-dump stdout --save rcvr.avi --transport udp
```

### Sender
Take a look at the `--cc-dump` and `--rtp-dump` options to generate additional log information.
```sh
# GCC
./roq send -a 127.0.0.1:4242 --source train_30.mp4 --codec h264 --save sndr.avi --transport udp --gcc

# SCReAM
./roq send -a 127.0.0.1:4242 --source train_30.mp4 --codec h264 --save sndr.avi --transport udp --scream

# static (will stream at 5 Mbps = 5000000 bits per second)
./roq send -a 127.0.0.1:4242 --source train_30.mp4 --codec h264 --save sndr.avi --transport udp --initial-bitrate 5000000
```

### Debugging
Start the program with `GST_DEBUG=*:3 ./roq ...` to get GStreamer-related logging output.
Increase the number up to 8 to get more fine-grained output.
A common problem is that not all required GStreamer elements are available.
