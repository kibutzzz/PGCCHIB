# Trabalho GB

**Integrantes:** Benjamin Vichel, Leonardo Ramos e Lucas Kappes

## Pré-requisitos

- CMake configurado para o projeto, conforme descrito no [README principal](../../README.md).
- Dependências instaladas: GLFW, GLM, GLAD e STB_IMAGE.

## Como executar

1. Compile o projeto:
```sh
mkdir build
cd build
cmake ..
cmake --build .
```
2. Execute o programa:
```sh
./TrabalhoGB
```

## Funcionamento

Por padrão, o jogo carrega o mapa de `./assets/maps/map15x15.txt` e os objetivos de `./assets/maps/objective_positions.txt`. Para alterar o mapa e os objetivos, basta modificar esses arquivos.

### Regras do jogo:
- O personagem não pode andar sobre a água.
- O jogo termina se o personagem pisar na lava.
- Para vencer, o personagem deve coletar todos os objetivos.
