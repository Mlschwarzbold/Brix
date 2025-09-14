# Trabalho pratico de Sisop II - Brix
## BRICS Instant payment system
### Sistema de tranferência bancária instantânea e eficiente entre clientes
## Etapa 1
### Servidor único, múltiplos clientes, suporte a concorrência e multi-thread
# Build
## Requisitos
### CMake instalado
### Algumas bibliotecas, se vira
## Comando para compilar projeto
### cmake --build build
#### (CMake talvez esteja meio sus)
## Comando para executar o projeto
### ./build/server/servidor \<porta\>
### ./build/client/cliente \<porta\>

## O que está sendo feito agora
### Brodacast UDP pra achar IP do servidor
