CC = g++
SRC = Server/ServerManager.cpp
OUT = servidor

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
