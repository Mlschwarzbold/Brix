BUILD_DIR="./build"
DEFAULT_PORT=4000

set -e

build() {
    build_type="Debug"
    if [ "$1" == "release" ]; then
        echo "- Building Release"
        build_type="Release"

        else echo "- Building Debug"
    fi

    mkdir -p "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$build_type"
    cd "$BUILD_DIR" || exit 1
    make
    cd ..
}

server() {
    port="${1:-$DEFAULT_PORT}"
    build "$@"
    clear
    print_ip
    echo
    "$BUILD_DIR/server/servidor" "$port"
}

client() {
    port="${1:-$DEFAULT_PORT}"
    build "$@"
    clear
    print_ip
    echo
    "$BUILD_DIR/client/cliente" "$port"
}

clean() {
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning build directory..."
        rm -rf "$BUILD_DIR" cmake_install.cmake CMakeCache.txt Makefile .cache CMakeFiles

    else
        echo "Nothing to clean."
    fi
}


print_ip() {
    network_device=$(ip route get 8.8.8.8 | awk '{print $5}')
    echo "##### Network Device: $network_device #####"
    echo "##### IP: $(ip -4 -o address show $network_device | awk '{print $4}') #####"
}

build_container() {
    docker build . -f Dockerfile -t brix
}

network() {
    # Create a custom Docker network (bridge)
    docker network inspect brix-net >/dev/null 2>&1 || \
    docker network create brix-net

    # Start tmux session
    SESSION="brix-session"
    tmux new-session -d -s $SESSION

    # Enable mouse mode inside tmux
    tmux set-option -t $SESSION mouse on

    # Run server in first pane
    tmux send-keys -t $SESSION "docker run --rm --network brix-net --name server -it brix server 4000" C-m

    # Split window vertically, run first client
    tmux split-window -h -t $SESSION
    tmux send-keys -t $SESSION:0.1 "docker run --rm --network brix-net --name client1 -it brix client 4000" C-m

    # Split again, run second client (below first client)
    tmux split-window -v -t $SESSION:0.1
    tmux send-keys -t $SESSION:0.2 "docker run --rm --network brix-net --name  client2 -it brix client 4000" C-m

    # Attach to session
    tmux attach -t $SESSION
}

# Dispatch based on the first argument
case "$1" in
    build)
        shift
        build "$@"
        ;;
    server)
        shift
        server "$@"
        ;;
    client)
        shift
        client "$@"
        ;;
    print_ip)
        print_ip
        ;;
    build_container)
        build_container
        ;;
    network)
        build_container
        network
        ;;
    clean)
        clean
        ;;
    *)
        echo "Usage: $0 {build [release]|server [port] [release]|client [port] [release]|build_container|network|show_ip|clean}"
        exit 1
        ;;
esac
