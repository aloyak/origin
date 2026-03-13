#!/bin/bash
echo "Building Project..."

WEB_BUILD=false
NO_SANDBOX=false

for arg in "$@"; do
    case $arg in
        --web) WEB_BUILD=true ;;
        --no-sandbox) NO_SANDBOX=true ;;
        *) echo "Unknown argument: $arg"; exit 1 ;;
    esac
done

if [ "$WEB_BUILD" = true ]; then
    echo "Target: Web"

    if ! command -v emcc &> /dev/null; then
        echo "Error: emcc not found. Source the Emscripten SDK first:"
        echo "  source /path/to/emsdk/emsdk_env.sh"
        exit 1
    fi

    mkdir -p build-web && cd build-web
    emcmake cmake ..
    emmake make -j$(nproc)

    echo ""
    echo "Web build complete!"
    echo "Serve with:  python3 -m http.server 8080 --directory build-web/game/"

else
    echo "Target: Native"

    mkdir -p build && cd build
    cmake .. 
    if [ "$NO_SANDBOX" = true ]; then
        cmake .. -DORIGIN_NO_SANDBOX=ON
    else
        cmake ..
    fi
    cmake --build . -- -j$(nproc)

    echo "Build complete!"
fi