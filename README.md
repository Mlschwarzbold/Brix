# Trabalho pratico de Sisop II - Brix

## BRICS Instant payment system

Sistema de tranferência bancária instantânea e eficiente entre clientes

## Etapa 1

### Servidor único, múltiplos clientes, suporte a concorrência e multi-thread

# Build

## Quais os requisitos para buildar e executar o projeto?

Esse projeto suporta nix para reproducibilidade da build.
Se tiver nix-shell (ou nixos), basta utilizar `nix-shell` para ter acesso a todas os requisitos da build.
Adicionalmente, se tiver o direnv configurado, `direnv allow` tem o mesmo efeito.

Caso contrário, a lista de dependencias pode ser encontrada no próprio arquivo `shell.nix`, mas não deve ser maior que gcc/g++ e CMake, opcionalmente com docker e tmux para rodar alguns dos utilitários.

No projeto, também se encontra um dockerfile contendo um container pronto para ser executado:

```sh
# Buildar a imagem do container (baseada em um debian slim)
docker build . -f Dockerfile -t brix

## Para rodar o servidor
docker run -it brix server [port] [Release]
## Para rodar o cliente
docker run -it brix client [port] [Release]
```

## Como compilar e executar o projeto?

O projeto vem com uma cli utilitária `brix.sh`, com vários comandos para compilação e execução do cliente e servidor. 

```sh
# Builda o projeto para debug (padrão) ou release
./brix.sh build [Release]

# Builda o projeto para debug (padrão) e executa o cliente/servidor na porta especificada (ou 4000, por padrão)
./brix.sh server [porta] [Release]
./brix.sh client [porta] [Release]
```

Fora a cli, o projeto usa e possui suporte a CMake.
