# Infraestrutura de Software - Implementação DEADLOCK

Algoritmo do Banker (evitar deadlock) em C: lê os clientes e os comandos de
arquivos e escreve o resultado em `result.txt`.

## Compilar e executar

```bash
make
./banker 10 5 7
```

Cada argumento é o número de instâncias de um tipo de recurso (aqui, 3
recursos com 10, 5 e 7 instâncias). O programa lê `customer.txt` e
`commands.txt` da pasta atual e grava `result.txt`.

Para apagar os arquivos compilados:

```bash
make clean
```

## Arquivos de entrada

**`customer.txt`**: um cliente por linha, com a demanda máxima de cada
recurso separada por vírgula. A quantidade de números deve ser igual em todas
as linhas e igual ao número de argumentos.

```
7,5,3
3,2,2
```

**`commands.txt`**: um comando por linha.

| Comando | Significado |
|---|---|
| `RQ c a b ...` | cliente `c` pede os recursos `a b ...` |
| `RL c a b ...` | cliente `c` devolve os recursos `a b ...` |
| `*` | imprime as tabelas MAXIMUM, ALLOCATION, NEED e AVAILABLE |

## Tratamento de erros

O programa mostra uma mensagem de erro e encerra (código de saída 1), sem
gravar `result.txt`, quando:

- o número de argumentos é diferente do número de recursos de `customer.txt`;
- um argumento não é um número inteiro maior ou igual a zero;
- `customer.txt` ou `commands.txt` não existe ou não pode ser aberto;
- `customer.txt` tem formato inválido (linhas com quantidades diferentes de
  números, texto, números negativos ou arquivo vazio);
- alguma linha de `commands.txt` é inválida (comando desconhecido, cliente
  inexistente, valores faltando, sobrando ou negativos). Todos os comandos
  são verificados antes de qualquer um ser executado.

## Estrutura

| Arquivo | Conteúdo |
|---|---|
| `banker.h`, `banker.c` | algoritmo do Banker: verificação de segurança, pedido, devolução e impressão das tabelas |
| `parser.h`, `parser.c` | leitura e validação de `customer.txt` e `commands.txt` |
| `main.c` | argumentos, execução dos comandos e mensagens de `result.txt` |
| `Makefile` | `make` gera `banker`; `make clean` remove os compilados |
| `evidencias.log` | registro dos testes feitos no terminal |
