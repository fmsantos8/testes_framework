# Resultados computacionais do passo 6

Valores extraídos dos oito builds identificados no passo 2, sem gravar ou executar placas durante a coleta das métricas em 02/11/2026. A campanha física posterior está registrada no [estado das evidências](../README.md); seus resultados não alteram esses números de código e memória.

## Código autoral original

| Projeto | C/H autorais | LOC total | Comum | Específico | Pcomp |
| --- | ---: | ---: | ---: | ---: | ---: |
| esp32_framework | 54 | 2.783 | 1.815 | 968 | 65,22% |
| esp32_sem_framework | 17 | 875 | 0 | 875 | 0,00% |
| nrf52840_framework | 54 | 2.572 | 1.815 | 757 | 70,57% |
| nrf52840_sem_framework | 18 | 969 | 0 | 969 | 0,00% |
| stm32f407_framework | 55 | 3.138 | 1.815 | 1.323 | 57,84% |
| stm32f407_sem_framework | 29 | 1.049 | 0 | 1.049 | 0,00% |
| stm32f411_framework | 55 | 3.138 | 1.815 | 1.323 | 57,84% |
| stm32f411_sem_framework | 29 | 1.049 | 0 | 1.049 | 0,00% |

O comum global compreende 44 C/H autorais e 1.815 LOC, usados a partir das mesmas origens nas quatro placas. O arquivo de fontes geradas participa da comparação de identidade, mas é excluído de LOC: os 45 C/H comuns finais são idênticos.

Nas versões convencionais, o Pcomp estrito é zero porque os arquivos estão mantidos em caminhos independentes. Há, porém, 11 arquivos autorais idênticos nas quatro versões, somando 486 LOC em cada uma. Isso evidencia conteúdo passível de reúso; zero na métrica de origem não significa ausência de lógica portável ou impossibilidade de compartilhá-la.

## Controle de formatação

Formatação uniforme foi aplicada somente a cópias, preservando os fontes compilados. Os dois resultados são publicados; LOC não é medida direta de complexidade ou esforço.

| Projeto | LOC original | LOC em cópia formatada | Pcomp em cópia formatada |
| --- | ---: | ---: | ---: |
| esp32_framework | 2.783 | 2.775 | 64,86% |
| esp32_sem_framework | 875 | 925 | 0,00% |
| nrf52840_framework | 2.572 | 2.656 | 67,77% |
| nrf52840_sem_framework | 969 | 1.041 | 0,00% |
| stm32f407_framework | 3.138 | 3.123 | 57,64% |
| stm32f407_sem_framework | 1.049 | 1.129 | 0,00% |
| stm32f411_framework | 3.138 | 3.123 | 57,64% |
| stm32f411_sem_framework | 1.049 | 1.129 | 0,00% |

## Código mantido no conjunto das quatro placas

| Forma | Soma dos checkouts | LOC por origem/conteúdo | LOC formatadas por origem/conteúdo | Arquivos/versões por origem |
| --- | ---: | ---: | ---: | ---: |
| framework | 11.631 | 5.106 | 5.197 | 79 |
| sem_framework | 3.942 | 3.942 | 4.224 | 93 |

O porte STM32 representa 7 arquivos e 1.080 LOC mantidos em uma origem e usados pelas duas placas. Ele é específico do ambiente STM32 na métrica global.

O conjunto com framework tem mais LOC autorais, mesmo após descontar as origens compartilhadas. A contribuição observada está no reúso e nas fronteiras de dependência; os resultados não demonstram redução do volume total de código, tempo de desenvolvimento ou esforço de manutenção.

## Dependências diretas dos SDKs

| Projeto | Arquivos com SDK / autorais | Percentual |
| --- | ---: | ---: |
| esp32_framework | 8 / 54 | 14,81% |
| esp32_sem_framework | 6 / 17 | 35,29% |
| nrf52840_framework | 9 / 54 | 16,67% |
| nrf52840_sem_framework | 7 / 18 | 38,89% |
| stm32f407_framework | 8 / 55 | 14,55% |
| stm32f407_sem_framework | 16 / 29 | 55,17% |
| stm32f411_framework | 8 / 55 | 14,55% |
| stm32f411_sem_framework | 16 / 29 | 55,17% |

Nos quatro projetos com framework não foram encontradas dependências diretas de SDK em aplicação, componentes, contratos ou arena. As referências estão nos portes, BSP e integração. Nas versões convencionais também aparecem em aplicação, dispositivos e telemetria. A contagem absoluta de arquivos com SDK é maior no framework em ESP32/nRF; percentuais e número de diretórios dependem da decomposição e não devem ser interpretados isoladamente como qualidade.

## Memória — bytes da aplicação

| Placa | Flash sem | Flash com | Acréscimo flash | RAM sem | RAM com | Acréscimo RAM |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| esp32 | 200.928 | 208.272 | 7.344 (3,65%) | 62.600 | 63.792 | 1.192 (1,90%) |
| nrf52840 | 85.300 | 90.724 | 5.424 (6,36%) | 16.640 | 18.432 | 1.792 (10,77%) |
| stm32f407 | 23.192 | 34.144 | 10.952 (47,22%) | 6.592 | 8.624 | 2.032 (30,83%) |
| stm32f411 | 23.100 | 33.844 | 10.744 (46,51%) | 6.592 | 8.624 | 2.032 (30,83%) |

Flash corresponde ao BIN da aplicação, com inicialização de RAM e empacotamento/alinhamento, excluindo bootloader separado e áreas externas à imagem. RAM soma as regiões estáticas/reservadas indicadas por ELF/MAP, incluindo noinit e reservas existentes, sem somar stacks/heaps duas vezes. Não representa pico em execução.

O framework aumentou flash e RAM nos quatro pares. Os maiores acréscimos relativos de flash ocorreram nos STM32. As diferenças abrangem a implementação completa, incluindo validações, contextos e capacidades dos portes; não isolam o custo de uma chamada abstrata.

## Dados e reprodução

Consulte [README.md](README.md) para regras, exclusões, arquivos e comandos. Os CSVs preservam números sem separador de milhar e com ponto decimal; esta síntese usa a apresentação em português.
