# Capacidades dos portes no recorte ensaiado

Esta declaração descreve o código vigente; os resultados de execução estão na [matriz](resultados/MATRIZ.md). Uma capacidade declarada não é uma aprovação física. A identificação exata do contrato e dos portes usa as revisões Git e os hashes registrados em [resultados.json](resultados/resultados.json).

## Contrato e ciclo de vida comuns

- Contrato público: `hal/interface`, revisão `21b0e0fa80245341ca3daca9ad5f7df0ab289dd8`, tag local `v1.0.0` nas quatro integrações. A tag identifica esta versão; não demonstra conformidade por si só.
- Implementações: ESP32 `0b7c43e15450fbeb8efacd3d6fbbab83e63cf1a8`; nRF52840 `3729cc2dc3f36f3ebb701c1377bba57430b75c0d`; STM32F4 `6ba6ffefbf2854645818d353fabfdbb998514760` nas duas placas.
- Inicialização de sistema única, buffer externo alinhado e persistente, construção sequencial e selagem idempotente. Sem destruição ou reconfiguração em execução. Falhas de construção podem consumir arena e não garantem rollback do hardware.
- GPIO opera em níveis físicos; a polaridade lógica chega pelo BSP. Entrada e saída push-pull; seleção de resistores internos conforme as capacidades do pino.
- I2C mestre, endereços de sete bits entre `0x08` e `0x77`, blocos não vazios, leitura e escrita síncronas separadas com STOP no fluxo normal. Não há requisito de repeated START.
- UART transmite de forma síncrona, devolve a quantidade aceita e admite transmissão vazia. Sucesso não confirma recepção remota. Em falha, bytes já aceitos podem ainda sair pelo fio. O porte não repete automaticamente a transmissão.
- Timeouts positivos são fixados na construção e controlam a espera do driver; não são um prazo estrito para a chamada inteira.
- Temporizador físico periódico, callback curto em interrupção e início explícito após inicializar as dependências. Não há consulta de milissegundos na HAL. Contar callbacks mede notificações atendidas, sujeito a atraso e aglutinação.
- Sem garantia geral de reentrância, uso concorrente do mesmo handle ou uso das transferências em ISR. A integração serializa o barramento e o estado compartilhado; não há abstração central de RTOS. Isso não impede o callback de temporizador explicitamente previsto em ISR.

## Capacidades por plataforma

| Aspecto | ESP32 | nRF52840 | STM32F407 / STM32F411 |
| --- | --- | --- | --- |
| SDK usado no firmware de produção | ESP-IDF 6.1 | Subconjunto fixado do nRF Connect SDK 3.1.0 | STM32CubeF4 1.28.3 |
| CPU | 80/160/240 MHz; com PM desabilitado deve coincidir com a frequência fixada no build | 64 MHz estabelecidos pelo Zephyr | F407: 80/84/100/168 MHz; F411: 80/84/100 MHz |
| I2C: controladores | IDs 0 e 1 | Somente ID 0, TWIM0 e rota do overlay | IDs 0/1/2 → I2C1/I2C2/I2C3; rotas validadas por MCU |
| I2C: frequência aceita pelo código | 10 a 400 kHz | 100, 250 ou 400 kHz | 10 a 400 kHz |
| I2C: tamanho máximo por chamada | 65.535 bytes | 256 bytes, com buffer persistente de EasyDMA | 65.535 bytes |
| I2C: timeout | Convertível para ticks positivos e finitos; limitado também a `INT_MAX` e ao overflow da conversão | 1 a `INT32_MAX` ms | 1 a `UINT32_MAX - 1` ms; `HAL_MAX_DELAY` rejeitado |
| UART: controladores | IDs 0 a 2, respeitando reserva de console/driver/pinos | Somente UARTE0 e rota do overlay | F407: IDs 0 a 5; F411: 0, 1 e 5; demais IDs válidos indisponíveis |
| UART: formatos | 7/8 bits de dados, paridade ausente/par/ímpar, 1/2 stops | 115200 8N1 | 8 bits com ou sem paridade; 7 bits exigem paridade; 1/2 stops |
| UART: tamanho e espera | Tamanho `uint32_t`; alimentação da FIFO e espera de término sob limite em ticks | Tamanho `uint32_t`, fracionado em blocos de até 64 bytes; abort após timeout tem espera adicional de até 10 ms | Até 65.535 bytes; timeout do driver ST |
| Temporizador: recursos | Quatro IDs lógicos, alocados pelo GPTimer | Somente ID 2 → TIMER2 | IDs 0/1/2/3 → TIM2/TIM3/TIM4/TIM5 |
| Temporizador: período | 1 a `UINT32_MAX` ms com resolução configurada de 1 MHz | 1 a 4.294.967 ms com contador de 32 bits a 1 MHz | Períodos representáveis exatamente por prescaler de 16 bits e ARR de 16 ou 32 bits; demais retornam `NOT_SUPPORTED` |

Os limites acima descrevem validação e representação no porte. Não prometem frequência elétrica exata nem sucesso de uma transferência de qualquer tamanho no timeout escolhido. As rotas e os pinos disponíveis fisicamente dependem do encapsulamento e da placa.

No ensaio host do ESP32, o modelo usa CPU de 160 MHz, FreeRTOS a 100 Hz, PM desabilitado e `CONFIG_GPTIMER_ISR_CACHE_SAFE` desabilitado. Esses valores correspondem ao `sdkconfig` do build de produção desta rodada. O timeout abaixo de um tick não é exercitado como uma espera de alta resolução. As variantes com PM ou cache-safe habilitados não foram ensaiadas.

O caso `TIMER-03` é `NAO_APLICAVEL` no modelo ESP32: na configuração ensaiada não há um período positivo de `uint32_t` declarado indisponível. IDs fora da faixa são argumentos inválidos, não substitutos para testar `NOT_SUPPORTED`.

## Seleção dos BSPs da prova de conceito

| Placa | CPU nominal | I2C a 100 kHz | UART a 115200 8N1 | Botão / LED | Temporizador |
| --- | --- | --- | --- | --- | --- |
| ESP32 DevKit V1 | 160 MHz | I2C0, SDA 21 / SCL 22 | UART0, TX 1 / RX 3 | GPIO18 / GPIO19 | GPTimer, ID 0, 1 ms |
| Pro Micro nRF52840/nice!nano v2 | 64 MHz | TWIM0, SDA P0.08 / SCL P0.11 | UARTE0, TX P0.20 / RX P0.22 | P0.06 / P0.17 | TIMER2, ID 2, 1 ms |
| DevEBox STM32F407 | 168 MHz | I2C1, SDA PB9 / SCL PB8 | USART2, TX PA2 / RX PA3 | PA0 / PA1 | TIM2, ID 0, 1 ms |
| Black Pill STM32F411 | 100 MHz | I2C1, SDA PB7 / SCL PB6 | USART2, TX PA2 / RX PA3 | PB0 / PB1 | TIM2, ID 0, 1 ms |

Nas quatro: SHT31-D em `0x44`, SSD1306 em `0x3C`, botão ativo em nível baixo com pull-up externo de 10 kΩ e LED ativo em nível alto com resistor de 1 kΩ. O timeout selecionado no BSP é 100 ms para I2C e 1.000 ms para UART. O caso `BSP-01` compila e executa o BSP real com o porte real sobre o ambiente de teste.

## Fontes da declaração

- [Interfaces públicas](../esp32_framework/src/hal/interface/).
- [Porte ESP32](../esp32_framework/src/hal/ports/esp32-wroom-32/), especialmente validações de I2C/UART e conversão de timeout em `hal_esp32.c`.
- [Porte nRF52840](../nrf52840_framework/src/hal/ports/nrf52840/), incluindo limites de buffers e timeout/abort da UART.
- [Porte STM32F4](../stm32f411_framework/src/hal/ports/stm32f4/), incluindo validação de formatos, rotas e cálculo do período.
- [BSP ESP32](../esp32_framework/src/bsp/esp32_devkit_v1/bsp.c), [BSP nRF52840](../nrf52840_framework/src/bsp/pro_micro_nrf52840/bsp.c), [BSP F407](../stm32f407_framework/src/bsp/devebox_stm32f407/bsp.c) e [BSP F411](../stm32f411_framework/src/bsp/black_pill_stm32f411/bsp.c).

Após alterar contrato, porte, BSP, SDK ou configuração relevante, repetir os ensaios afetados e revisar esta declaração. Isso não requer editar a proposta do TCC1.
