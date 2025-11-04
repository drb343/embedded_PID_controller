# Embedded Linux PID Controller

This project implements a simple PID controller compiled for ARM Linux using a Docker-based cross-compilation environment.

## PID Output

![PID Plot](docs/pid_plot.png)

## How to Run

Clone the repository and enter the project directory:
```bash
git clone https://github.com/drb343/embedded_linux.git
cd embedded_linux
```

Start the Docker container:
```bash
docker run --rm -it     -v $(pwd):/workspace     -e DISPLAY=:0     arm-cross bash
```

Build and run the program:
```bash
cd src
make clean
make CROSS_COMPILE=arm-linux-gnueabihf- LDFLAGS="-static -lm"
./pid_controller
```

## Requirements
- Docker  
- ARM GNU toolchain (inside Docker)  
- Make

## Author
Denis Brown
