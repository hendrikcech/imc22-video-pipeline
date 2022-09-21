#!/usr/bin/env sh

set -eu

GST_SNDR_OPTIONS=$(cat <<-EOF
    -Dbuildtype=release
    -Dauto_features=disabled
    -Dbase=enabled
    -Dgood=enabled
    -Dbad=enabled
    -Dgst-plugins-base:videotestsrc=enabled
    -Dgst-plugins-base:tcp=enabled
    -Dgst-plugins-base:app=enabled
    -Dgst-plugins-good:rtp=enabled
    -Dgst-plugins-good:udp=enabled
    -Dgst-plugins-base:playback=enabled
    -Dgst-plugins-bad:videoparsers=enabled
    -Dgst-plugins-base:videoconvert=enabled
    -Dgst-plugins-good:avi=enabled
    -Dgst-plugins-good:isomp4=enabled
    -Dlibav=enabled
    -Dgpl=enabled
    -Dugly=enabled
    -Dgst-plugins-ugly:x264=enabled
EOF
)

GST_RCVR_OPTIONS=$(cat <<-EOF
    -Dauto_features=disabled
    -Dbuildtype=release
    -Dbase=enabled
    -Dbad=enabled
    -Dgood=enabled
    -Dgst-plugins-base:tcp=enabled
    -Dgst-plugins-base:playback=enabled
    -Dgst-plugins-base:videoconvert=enabled
    -Dgst-plugins-base:app=enabled
    -Dgst-plugins-bad:debugutils=enabled
    -Dgst-plugins-bad:videoparsers=enabled
    -Dgst-plugins-good:rtp=enabled
    -Dgst-plugins-good:rtpmanager=enabled
    -Dgst-plugins-good:udp=enabled
    -Dgst-plugins-good:avi=enabled
    -Dgst-plugins-good:videobox=enabled
    -Dgst-plugins-good:videocrop=enabled
    -Dlibav=enabled
    -Dgstreamer:coretracers=enabled
EOF
)

EXPECTED_GST_PLUGINS=$(cat <<-EOF
gst-plugins-bad 1.20.1

    Plugins               : debugutilsbad, videoparsersbad
    (A)GPL license allowed: True

gst-plugins-base 1.20.1

    Plugins: app, playback, tcp, videoconvert, videotestsrc

gst-plugins-good 1.20.1

    Plugins: avi, isomp4, rtp, rtpmanager, udp, videobox, videocrop

gst-plugins-ugly 1.20.1

    Plugins               : x264
    (A)GPL license allowed: True

gstreamer 1.20.1

    Plugins: coreelements, coretracers
EOF
)

clone_gstreamer() {
    git clone --depth=1 -b 1.20.1 https://gitlab.freedesktop.org/gstreamer/gstreamer.git
}

configure_gstreamer() {
    meson $GST_SNDR_OPTIONS $GST_RCVR_OPTIONS builddir
    printf '\n\n\n'
    echo "Check if all the following plugins are also enabled for your build."
    echo "If some are missing, check the preceding output. You are most likely"
    echo "missing a dependency."
    echo
    echo "$EXPECTED_GST_PLUGINS"
}

build_gst_timecode() {
   git clone https://github.com/hendrikcech/gst-timecode.git
   cd gst-timecode
   meson builddir
   ninja -C builddir
   cd ..
}

setup_go() {
    wget https://go.dev/dl/go1.17.13.linux-amd64.tar.gz
    tar -xzf go1.17.13.linux-amd64.tar.gz
}

build_roq() {
    go/bin/go build -o roq
}

prompt_user() {
    # https://stackoverflow.com/a/226724
    while true; do
        read -p "$1 [y/n] " yn
        case $yn in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer yes or no.";;
        esac
done
}

check_env() {
    for ELEMENT in timecodeoverlay avdec_h264 h264parse qtdemux; do
        if [ "$(gst-inspect-1.0 | grep "$ELEMENT" | wc -l)" -eq 0 ]; then
            echo "WARNING: GStreamer element $ELEMENT is not available. Something during"
            echo "the GStreamer compilation or the environment setup went wrong."
            # exit 1
        fi
    done
}

print_usage() {
    echo "Everything's set up! The application is available at ./roq"
    echo
    echo "Sample usage:"
    echo "Start the receiver in this terminal:"
    echo "./roq receive -a :4242 --sink fpsdisplaysink --fps-dump stdout --save rcvr.avi --transport udp --rfc8888"
    echo
    echo "Open another terminal, execute setup_environment.sh again and start the sender."
    echo "Make sure to change the path to '--source':"
    echo "./roq send -a 127.0.0.1:4242 --source train_30.mp4 --codec h264 --save sndr.avi --transport udp --scream"
}

main() {
    BASE_DIR="$(dirname "$0")"
    cd "$BASE_DIR"
    BASE_DIR="$(pwd)"

    if [ ! -d "$BASE_DIR/gstreamer" ]; then
        echo '"gstreamer/" missing: Cloning gstreamer'
        clone_gstreamer
    else
        echo 'GStreamer already cloned'
    fi

    if [ ! -d "$BASE_DIR/gstreamer/builddir" ]; then
        echo '"gstreamer/builddir" missing: Configuring and building GStreamer'
        cd "$BASE_DIR/gstreamer"
        configure_gstreamer
        if ! prompt_user "Configuration looks correct? Continue?"; then
            exit 1
        fi
        echo "Compiling gstreamer"
        ninja -C builddir
        cd "$BASE_DIR"
    else
        echo 'Assuming that GStreamer was successfully built.'
        echo 'Remove "gstreamer/builddir" to compile it again.'
    fi

    echo "Setting up the GStreamer environment"
    eval "$("$BASE_DIR/gstreamer/gst-env.py" --only-environment)"

    if [ ! -d "$BASE_DIR/gst-timecode" ]; then
        echo "Building gst-timecode"
        build_gst_timecode
    else
        echo 'Assuming that gst-timecode was successfully built.'
        echo 'Remove "gstreamer/gst-timecode" to compile it again.'
    fi
    export GST_PLUGIN_PATH="$GST_PLUGIN_PATH:${BASE_DIR}/gst-timecode/builddir"

    if [ ! -d 'go' ]; then
        echo 'Downloading go to build roq'
        setup_go
    else
        echo 'go already downloaded'
    fi
    build_roq

    check_env

    print_usage
    cd "$BASE_DIR"
    bash
}

main
