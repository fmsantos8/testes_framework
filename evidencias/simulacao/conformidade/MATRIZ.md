# Resultados de conformidade dos portes

Gerado por `python3 conformidade/run.py`. Execução UTC: 2026-11-03T00:45:00.605112+00:00.

Resultados simulados não comprovam operação física ou temporização real.

| Caso | Objetivo | ESP32 | nRF52840 | STM32F407 | STM32F411 |
| --- | --- | --- | --- | --- | --- |
| SYS-01 | Recursos antes da inicialização | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| SYS-02 | Argumentos de sistema, frequência indisponível e nova tentativa válida | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| SYS-03 | Inicialização única, selagem idempotente e uso posterior | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| SYS-04 | Arena insuficiente sem publicar handles | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| CFG-01 | Ponteiros e configurações inválidos sem consumir arena | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| CFG-02 | Configurações válidas indisponíveis retornam NOT_SUPPORTED | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| CFG-03 | Pino, par de pinos, enumeração e identificador inválidos | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| RES-01 | Reserva exclusiva de pinos e controladores | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| GPIO-01 | Níveis físicos, direção e argumentos de GPIO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| I2C-01 | Endereço reservado, buffers, tamanho zero e limite do porte | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| I2C-02 | Escrita/leitura separadas, endereço e timeout configurado | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| I2C-03 | Tradução de erro de comunicação e chamada seguinte | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| I2C-04 | Tradução de timeout e chamada seguinte | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| I2C-05 | Falha de construção no SDK sem publicação de handle | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-01 | Argumentos, contagem inicial e transmissão vazia | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-02 | Transmissão completa com contagem de bytes | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-03 | Timeout sem progresso | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-04 | Timeout com prefixo de dois bytes | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-05 | Erro de transmissão sem progresso | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| UART-06 | Bloco de 129 bytes, incluindo fracionamento interno no nRF | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| TIMER-01 | Handle nulo e callback obrigatório | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| TIMER-02 | Sem callback antes do início, início após selagem e início repetido | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| TIMER-03 | Período ou recurso válido indisponível | NAO_APLICAVEL | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |
| BSP-01 | Composição real da placa, polaridades, arena selada e temporizador parado | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO | APROVADO_SIMULADO |

## Ensaios físicos

| Caso | Objetivo | Estado nas quatro placas |
| --- | --- | --- |
| HW-01 | Confirmar níveis elétricos, polaridades e estado inicial do LED | NAO_EXECUTADO |
| HW-02 | Capturar I2C: 100 kHz, endereços, STOP e intervalo de conversão | NAO_EXECUTADO |
| HW-03 | Provocar NACK e barramento retido após partida; registrar códigos e retomada | NAO_EXECUTADO |
| HW-04 | Capturar UART 115200 8N1, conteúdo e fim da transmissão | NAO_EXECUTADO |
| HW-05 | Medir temporizador de 1 ms, primeiro evento e perda/aglutinação sob carga | NAO_EXECUTADO |
| HW-06 | Exercitar UART parcial/timeout real com instrumentação ou registrar limitação | NAO_EXECUTADO |

Revisões, hashes, códigos de saída e caminhos dos logs: [resultados.json](resultados.json).
Procedimento e limitações: [roteiro](../../../conformidade/README.md).

## Evidência estrutural e de compilação

- Identidade dos arquivos comuns: True.
- Identidade do porte STM32F4: True.
- Cabeçalhos públicos compilados em C11/C++11 sem diretórios dos SDKs; resultados por alvo no JSON.
- Builds de produção conferidos por hashes e por correspondência com os fontes ensaiados: 4.

| Build de produção | Resultado |
| --- | --- |
| stm32f407_framework | APROVADO_COMPILACAO |
| stm32f411_framework | APROVADO_COMPILACAO |
| esp32_framework | APROVADO_COMPILACAO |
| nrf52840_framework | APROVADO_COMPILACAO |
