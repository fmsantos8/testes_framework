# Implementações convencionais da prova de conceito

Guia da implementação e de sua sessão inicial de validação. A campanha física posterior, realizada em 07–08/11/2026, está em [evidencias/README.md](../evidencias/README.md). Os caminhos de compilação apresentados abaixo são relativos à raiz do conjunto de projetos.

Foram implementados quatro projetos independentes: [ESP32](https://github.com/fmsantos8/esp32_sem_framework/blob/aadda2b6c88c49c3c5a2c9881eed44b81a0695e7/README.md), [nRF52840](https://github.com/fmsantos8/nrf52840_sem_framework/blob/2b5f18c60d5a35708b953ae3cd9cbf9078e97bce/README.md), [STM32F407](https://github.com/fmsantos8/stm32f407_sem_framework/blob/7e9dbaec81ad11c1e2643db8164e476dea00bbfc/README.md) e [STM32F411](https://github.com/fmsantos8/stm32f411_sem_framework/blob/e7dec8b4808b5868ee5d2069c0b012060c69c718/README.md).

“Sem framework” significa sem o framework de portabilidade desenvolvido no TCC. Os SDKs dos fabricantes continuam presentes: ESP-IDF 6.1.x, STM32CubeF4 HAL v1.28.3 e o subconjunto de execução do nRF Connect SDK v3.1.0. Os projetos não incluem fontes, bibliotecas, submódulos, contratos `hal_*`, arena, BSP nem componentes `cmp_*` dos projetos com framework. Cada árvore pode ser compilada separadamente.

## Organização e dependências

| Plataforma | Estrutura e integração |
| --- | --- |
| STM32F407 / STM32F411 | `Core/Inc`, `Core/Src`, `Core/Startup`, `Drivers/CMSIS`, `Drivers/STM32F4xx_HAL_Driver`; `main`, `SystemClock_Config`, `MX_*_Init`, MSP, handlers de interrupção, callback TIM, startup e linker próprios de cada MCU. A família correta dos drivers é **STM32F4xx**. |
| ESP32 | Projeto ESP-IDF com `main/CMakeLists.txt`, `app_main`, `sdkconfig.defaults`, handles nativos I2C, UART0 e GPTimer. |
| nRF52840 | Aplicação Zephyr/NCS com `src`, `prj.conf`, configurações Debug/Release e overlay devicetree; GPIO/pinctrl/clock do Zephyr e TWIM0/UARTE0/TIMER2 do nrfx. |

Nas versões convencionais, `application.c` reúne o fluxo da prova de conceito e acessa GPIO diretamente pelo SDK. Os drivers de sensor e OLED acessam o barramento nativo fixo da placa. A UART é controlada diretamente em `telemetry.c` ou pelos helpers nrfx específicos de `peripherals.c`. Não foi criado outro contrato de transporte comum ou mecanismo de injeção de dependências.

As rotinas de desenho em RAM, fontes, ícones e layout foram derivadas dos fontes existentes, preservando atribuições. Essas cópias locais garantem igualdade visual e não vinculam os builds ao framework. Sua origem e identidade devem ser explicitadas na análise de reúso: cópias independentes não constituem uma única origem compartilhada. Código ST, startup, CMSIS, syscalls, linker e demais artefatos de fabricante/gerados devem receber as exclusões previstas para as métricas.

## Comportamento preservado

| Item | Política |
| --- | --- |
| Inicialização | LED externo apagado; aquecedor do SHT31-D desligado; display configurado, limpo e ligado. Falha nessa fase interrompe a execução. |
| Sensor | SHT31-D em `0x44`, comando `0x2400`, alta repetibilidade, sem clock stretching. |
| Aquisição | Primeira tentativa imediata; novas tentativas após 2000 notificações contadas desde o retorno do comando anterior. Leitura após pelo menos 16 notificações desde o retorno do comando. |
| Integridade | Validação dos dois CRCs, polinômio `0x31`, inicialização `0xFF`. Temperatura em miligraus Celsius e umidade em milésimos de percentual, com a mesma conversão inteira e arredondamento. |
| Erros do sensor | Preserva a última amostra válida. Timeout e falha de comunicação ativam o erro I2C; uma leitura completa com CRC válido ou inválido limpa esse erro de comunicação. |
| Botão | Externo, ativo baixo, pull-up externo; debounce de 30 notificações; alterna uma vez por borda de pressionamento estável. |
| OLED | SSD1306 `0x3C`, 128×64, mesmo framebuffer, fontes, ícones, arredondamento e layout. Tela de leituras e tela de diagnóstico com estado da aquisição e uptime acumulado em 64 bits. |
| Atualização | No máximo uma página suja de 128×8 por passagem. Snapshot preservado até completar o envio; falhas mantêm a página pendente e adiam a tentativa por 1000 notificações. |
| LED | Aceso após inicialização, pisca com transições de 500 notificações quando sensor ou display têm erro de comunicação. Fontes de falha são independentes; sucesso em um dispositivo não apaga a falha do outro. |
| UART | 115200 8N1; fila de oito registros; novos eventos são descartados se a fila estiver cheia. Erros transitórios sem prefixo são repetidos após 1000 notificações. Registro parcialmente aceito é descartado e um delimitador CRLF deve ser aceito antes do próximo registro. |
| I2C | 100 kHz, pull-ups externos, operações síncronas separadas, cada uma encerrada com STOP e timeout de 100 ms. |
| Tempo | Temporizador físico com interrupção nominal de 1 ms, iniciado depois dos dispositivos e da aplicação. Contador de notificações atendidas, sujeito a latência e aglutinação; não se afirma precisão de relógio físico. |

A UART utiliza orçamento de 1000 ms; o nRF mantém também a espera limitada de 10 ms para abort. Nos STM32 há um SysTick separado para os timeouts da HAL ST. ESP32 e STM32 processam o laço continuamente; nRF cede ao SDK com `k_msleep(1)`, como as respectivas referências.

## Formato dos registros

Todos os registros terminam com CRLF:

```text
SAMPLE;temperature_mC=24800;humidity_mpercent=58300
ERROR;source=SHT31D;code=5
ERROR;source=SSD1306;code=5
```

Os códigos de dispositivo mantêm os valores da referência, independentemente dos códigos internos do SDK:

| Resultado | SHT31-D | SSD1306 |
| --- | ---: | ---: |
| Sucesso | 0 | 0 |
| Argumento inválido | 1 | 1 |
| Não inicializado | 2 | 2 |
| Estado inválido | 3 | — |
| Não suportado | 4 | 3 |
| Timeout | 5 | 4 |
| Comunicação | 6 | 5 |
| CRC inválido | 7 | — |

## Validação e artefatos

A [suíte de equivalência](../validacao_sem_framework/README.md) compara as quatro versões convencionais com os fontes reais do núcleo comum do framework. Ela verifica todos os bytes I2C/UART e transições do LED em três cenários por placa, incluindo falhas, saturação de fila e retorno do contador a zero. Compilação com avisos tratados como erros e UndefinedBehaviorSanitizer.

Evidências desta implementação:

- `build/validacao_sem_framework/results.json`: resultados das 12 comparações, hashes dos fontes e traces completas.
- `build/compilacao_sem_framework/manifesto.json`: sete builds — ESP32 padrão; nRF52840, STM32F407 e STM32F411 em Debug e Release —, comandos, SDKs, logs e hashes dos binários.
- Artefatos graváveis em `build/` de cada projeto, com instruções nos respectivos READMEs.

As referências existentes foram preservadas. `ANALISE.md` e `PROMPT.md` registram a situação anterior e não foram reescritos como se os novos projetos já existissem naquela rodada.

A simulação não substitui ensaios físicos. Não havia placas seriais disponíveis nesta sessão; não foram realizadas gravações nem validação elétrica, de interrupções ou de precisão temporal. Para a comparação experimental, executar o mesmo roteiro nas duas versões de cada placa, após inicialização bem-sucedida: aquisição válida, duas telas e botão, UART, repetição, desconexão/reconexão do sensor e do display, preservação da última amostra, indicação e recuperação. Registrar placa/bootloader, firmware e hash, SDK, flags e instrumentação. Comparações de memória devem usar configurações pareadas equivalentes; o manifesto fornece rastreabilidade, sem atribuir resultados físicos aos builds.

As estruturas seguem as convenções dos ambientes documentados pelo [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/build-system.html), [Zephyr](https://docs.zephyrproject.org/latest/develop/application/index.html) e [STM32CubeF4](https://github.com/STMicroelectronics/STM32CubeF4).
