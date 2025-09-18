BUILD_DIR="./build"
DEFAULT_PORT=4000

build() {
    build_type="Debug"
    if [ "$1" == "release" ]; then
        build_type="Release"
    fi

    mkdir "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$build_type"
    cd "$BUILD_DIR" || exit 1
    make
    cd ..
}

server() {
    port="${1:-$DEFAULT_PORT}"
    shift
    build "$@"
    "$BUILD_DIR/server/servidor" "$port"
}

client() {
    port="${1:-$DEFAULT_PORT}"
    shift
    build "$@"
    "$BUILD_DIR/client/cliente" "$port"
}

clean() {
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    else
        echo "Nothing to clean."
    fi
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
    clean)
        clean
        ;;
    *)
        echo "Usage: $0 {build [release]|server [port] [release]|client [port] [release]|clean}"
        exit 1
        ;;
esac
