BUILD_DIR="./build"
DEFAULT_PORT=4000
DEFAULT_BUILD_TYPE="Debug"

set -e

build() {
    build_type="${1:-$DEFAULT_BUILD_TYPE}"
    echo "- Building $build_type"

    cmake -S . -B build
    cmake --build build --target servidor -- -j$(nproc)

    mkdir -p "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$build_type"
    cd "$BUILD_DIR" || exit 1
    make -j 5
    cd ..
}

server() {
    port="${1:-$DEFAULT_PORT}"
    shift
    build "$@"
    clear
    print_ip
    echo
    "$BUILD_DIR/server/servidor" "$port"
}

client() {
    port="${1:-$DEFAULT_PORT}"
    shift
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

reset_network(){
    echo "ending and removing existing tmux session..."
    pkill -f tmux  || true
    echo "removing existing docker network..."
    docker ps -q | xargs -r -n1 docker kill

}

network() {
    build_type="${1:-$DEFAULT_BUILD_TYPE}"

    echo $build_type

    # Create a custom Docker network (bridge)
    docker network inspect brix-net >/dev/null 2>&1 || \
    docker network create brix-net

    # Start tmux session
    SESSION="brix-session"
    tmux new-session -d -s $SESSION

    # Enable mouse mode inside tmux
    tmux set-option -t $SESSION mouse on

    # Run server in first pane
    tmux send-keys -t $SESSION:0.0 "docker run --rm --network brix-net --name server1 -it brix server 4000 $build_type" C-m
    
    # Split window vertically orizontally, run first client
    tmux split-window -v -t $SESSION:0.0
    tmux send-keys -t $SESSION:0.1 "sleep 1 && docker run --rm --network brix-net --name client1 -it brix client 4000 $build_type" C-m

    # Split window vertically, run first client
    tmux split-window -h -t $SESSION:0.0
    tmux send-keys -t $SESSION:0.1 "sleep 1 && docker run --rm --network brix-net --name client2 -it brix client 4000 $build_type" C-m

    # Split again, run second client (below first client)
    tmux split-window -h -t $SESSION:0.1
    tmux send-keys -t $SESSION:0.2 "sleep 2 && docker run --rm --network brix-net --name server2 -it brix server 4000 $build_type" C-m

    # Attach to session
    tmux select-layout -t "$SESSION" tiled
    tmux attach -t $SESSION
}

election() {
    build_type="${1:-$DEFAULT_BUILD_TYPE}"

    echo $build_type

    # Create a custom Docker network (bridge)
    docker network inspect brix-net >/dev/null 2>&1 || \
    docker network create brix-net

    # Start tmux session
    SESSION="brix-session"
    tmux new-session -d -s $SESSION

    # Enable mouse mode inside tmux
    tmux set-option -t $SESSION mouse on

    # Run server in first pane
    tmux send-keys -t $SESSION:0.0 "docker run --rm --network brix-net --name server1 -it brix server 4000 $build_type" C-m
    
    # Split window vertically orizontally, run first client
    tmux split-window -v -t $SESSION:0.0
    tmux send-keys -t $SESSION:0.1 "sleep 1 && docker run --rm --network brix-net --name client1 -it brix client 4000 $build_type" C-m

    # Split window vertically, run third server
    tmux split-window -h -t $SESSION:0.0
    tmux send-keys -t $SESSION:0.1 "sleep 3 && docker run --rm --network brix-net --name server3 -it brix server 4000 $build_type" C-m

    # Split again, run second server
    tmux split-window -h -t $SESSION:0.1
    #tmux send-keys -t $SESSION:0.2 "sleep 2 && docker run --rm --network brix-net --name server2 -it brix server 4000 $build_type" C-m

    # Attach to session
    tmux select-layout -t "$SESSION" tiled
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
        shift
        build_container
        network "$@"
        ;;
    clean)
        clean
        ;;
    reset_network)
        reset_network
        ;;
    election)
        shift
        build_container
        election "$@"
        ;;
    *)
        echo "Usage: $0 {build [release]|server [port] [release]|client [port] [release]|build_container|network|election|show_ip|clean|reset_network}"
        exit 1
        ;;
esac
