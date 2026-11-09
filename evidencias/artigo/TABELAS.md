# Tabelas para o artigo do TCC2

Fechamento do passo 7 em 09/11/2026. Valores conferidos contra os registros de origem. Com/Sem indicam presença/ausência do framework. As versões finais em LaTeX estão em `tcc_2/tabelas/`; cada arquivo contém somente o ambiente `tabular`, pronto para receber legenda e posição no passo 8.

## Funcionamento das oito versões

| Placa | Versão | I | L | B | U | FS | RS | FD | RD |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ESP32 | Com | A | A | A | A | A | A | A | A |
| ESP32 | Sem | A | A | A | A | A | A | A | A |
| nRF52840 | Com | A | A | A | A | A | A | A | A |
| nRF52840 | Sem | A | A | A | A | A | A | A | A |
| STM32F407 | Com | A | A | A | A | A | A | A | A |
| STM32F407 | Sem | A | A | A | A | A | A | A | A |
| STM32F411 | Com | A | A | A | A | A | A | A | A |
| STM32F411 | Sem | A | A | A | A | A | A | A | A |

A = APROVADO, informado pelo responsável nas sessões de 07–08/11/2026. I: inicialização; L: leituras; B: botão/telas; U: UART; FS/RS: falha/recuperação do sensor; FD/RD: falha/recuperação do display. Os 64 campos aprovados resumem o recorte ensaiado. No STM32F407 sem framework, a recuperação do OLED inclui uma manipulação adicional das conexões e erros simultâneos; consultar LIMITACOES.csv e as observações originais.

Fonte: [../resultados-funcionais.csv](../resultados-funcionais.csv). LaTeX: [funcionamento.tex](../../tcc_2/tabelas/funcionamento.tex).

## Conformidade física no recorte observado

| Placa | GPIO | I2C | UART | TIMER |
| --- | --- | --- | --- | --- |
| ESP32 | A | A | A | L |
| nRF52840 | A | A | A | L |
| STM32F407 | A | A | A | L |
| STM32F411 | A | A | A | L |

A = APROVADO_FISICO somente para os casos observados; L = LIMITADO. GPIO: níveis e comportamento; I2C: endereços, comando 24 00, STOP, leitura separada e falha/retomada; UART: conteúdo e CR LF decodificados a 115200 8N1. TIMER: LED, evidência indireta; interrupções de 1 ms não medidas. A remoção integral dos módulos interrompe alimentação e comunicação. Não representa aprovação física de todos os contratos.

Fonte: [../resultados-conformidade.csv](../resultados-conformidade.csv). LaTeX: [conformidade-fisica.tex](../../tcc_2/tabelas/conformidade-fisica.tex).

## Casos simulados por grupo

| Placa | GPIO S/NA | I2C S/NA | UART S/NA | TIMER S/NA |
| --- | --- | --- | --- | --- |
| ESP32 | 1/0 | 5/0 | 6/0 | 2/1 |
| nRF52840 | 1/0 | 5/0 | 6/0 | 3/0 |
| STM32F407 | 1/0 | 5/0 | 6/0 | 3/0 |
| STM32F411 | 1/0 | 5/0 | 6/0 | 3/0 |

Cada célula mostra APROVADO_SIMULADO/NAO_APLICAVEL para casos distintos do respectivo grupo, por placa; não são repetições estatísticas. SYS, CFG, RES e BSP ficam na matriz completa. No conjunto completo: 95 aprovações e uma não aplicável. TIMER-03 não se aplica ao ESP32. A simulação usa fontes reais dos portes e substitutos do SDK no computador; não executa os binários embarcados. A seção HW da matriz simulada preserva o estado histórico de 02/11; os resultados físicos posteriores estão na tabela anterior e no CSV da campanha.

Fonte: [../simulacao/conformidade/resultados.json](../simulacao/conformidade/resultados.json). LaTeX: [conformidade-simulada.tex](../../tcc_2/tabelas/conformidade-simulada.tex).

## Código autoral e dependências diretas

| Placa | Versão | C/H | LOC | Comum | Origem (%) | Idêntico (%) | C/H SDK | SDK (%) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ESP32 | Com | 54 | 2.783 | 1.815 | 65,22% | 65,22% | 8 | 14,81% |
| ESP32 | Sem | 17 | 875 | 0 | 0,00% | 55,54% | 6 | 35,29% |
| nRF52840 | Com | 54 | 2.572 | 1.815 | 70,57% | 70,57% | 9 | 16,67% |
| nRF52840 | Sem | 18 | 969 | 0 | 0,00% | 50,15% | 7 | 38,89% |
| STM32F407 | Com | 55 | 3.138 | 1.815 | 57,84% | 57,84% | 8 | 14,55% |
| STM32F407 | Sem | 29 | 1.049 | 0 | 0,00% | 46,33% | 16 | 55,17% |
| STM32F411 | Com | 55 | 3.138 | 1.815 | 57,84% | 57,84% | 8 | 14,55% |
| STM32F411 | Sem | 29 | 1.049 | 0 | 0,00% | 46,33% | 16 | 55,17% |

LOC = coluna code do cloc 2.06 nos C/H autorais usados nos builds. Origem (%) corresponde a Pcomp = 100 × comum/(comum + específico), com origem mantida comum às quatro placas. Idêntico (%) considera o mesmo conteúdo, inclusive cópias em origens independentes. Nas versões convencionais, Pcomp = 0 por manutenção em caminhos independentes; existem 11 arquivos autorais idênticos, 486 LOC por versão. No framework, são 44 C/H autorais comuns e 1.815 LOC; o 45º C/H comum é fonte gráfica gerada, excluída de LOC e incluída no binário. Dependência direta: inclusão, tipo ou chamada de SDK revisada no inventário; a contagem absoluta é maior no framework em ESP32/nRF. As dependências ficam nos portes, BSP e integração; nas convencionais também aparecem na aplicação, dispositivos e telemetria.

Fonte: [../metricas/codigo.csv](../metricas/codigo.csv). LaTeX: [codigo-dependencias.tex](../../tcc_2/tabelas/codigo-dependencias.tex).

## Controle da sensibilidade à formatação

| Placa | Versão | LOC originais | LOC formatadas | Pcomp formatado |
| --- | --- | --- | --- | --- |
| ESP32 | Com | 2.783 | 2.775 | 64,86% |
| ESP32 | Sem | 875 | 925 | 0,00% |
| nRF52840 | Com | 2.572 | 2.656 | 67,77% |
| nRF52840 | Sem | 969 | 1.041 | 0,00% |
| STM32F407 | Com | 3.138 | 3.123 | 57,64% |
| STM32F407 | Sem | 1.049 | 1.129 | 0,00% |
| STM32F411 | Com | 3.138 | 3.123 | 57,64% |
| STM32F411 | Sem | 1.049 | 1.129 | 0,00% |

Contagens feitas em cópias com a mesma formatação; os fontes compilados foram preservados. Pcomp continua usando origem compartilhada. LOC não mede diretamente complexidade, qualidade ou esforço.

Fonte: [../metricas/codigo.csv](../metricas/codigo.csv). LaTeX: [controle-formatacao.tex](../../tcc_2/tabelas/controle-formatacao.tex).

## Código mantido por origem

| Forma | LOC checkouts | LOC por origem | LOC formatadas por origem | Arquivos/versões |
| --- | --- | --- | --- | --- |
| Com | 11.631 | 5.106 | 5.197 | 79 |
| Sem | 3.942 | 3.942 | 4.224 | 93 |

Origens são contadas uma única vez com seu conteúdo. Cópias convencionais idênticas permanecem origens separadas. O porte STM32 é compartilhado pelas duas placas: sete C/H e 1.080 LOC, específicos do ambiente STM32, fora do comum às quatro placas.

Fonte: [../metricas/resultados.json](../metricas/resultados.json). LaTeX: [codigo-mantido.tex](../../tcc_2/tabelas/codigo-mantido.tex).

## Flash da imagem da aplicação

| Placa | Sem (B) | Com (B) | Acréscimo (B) | Acréscimo (%) |
| --- | --- | --- | --- | --- |
| ESP32 | 200.928 | 208.272 | 7.344 | 3,66% |
| nRF52840 | 85.300 | 90.724 | 5.424 | 6,36% |
| STM32F407 | 23.192 | 34.144 | 10.952 | 47,22% |
| STM32F411 | 23.100 | 33.844 | 10.744 | 46,51% |

Bytes do BIN da aplicação, incluindo dados inicializados e empacotamento/alinhamento. Bootloader separado e áreas externas à imagem excluídos. Acréscimos em relação à versão sem framework, calculados diretamente dos bytes inteiros e arredondados uma única vez para apresentação. ESP32: default (-Og); nRF/STM32: Debug (-Og).

Fonte: [../metricas/comparacao-memoria.csv](../metricas/comparacao-memoria.csv). LaTeX: [flash.tex](../../tcc_2/tabelas/flash.tex).

## RAM estática e reservada

| Placa | Sem (B) | Com (B) | Acréscimo (B) | Acréscimo (%) |
| --- | --- | --- | --- | --- |
| ESP32 | 62.600 | 63.792 | 1.192 | 1,90% |
| nRF52840 | 16.640 | 18.432 | 1.792 | 10,77% |
| STM32F407 | 6.592 | 8.624 | 2.032 | 30,83% |
| STM32F411 | 6.592 | 8.624 | 2.032 | 30,83% |

Bytes estáticos/reservados por ELF/MAP, incluindo noinit e reservas existentes de stack/heap sem dupla contagem. Não mede pico durante a execução. Os acréscimos abrangem as implementações completas e não isolam o custo de uma chamada abstrata.

Fonte: [../metricas/comparacao-memoria.csv](../metricas/comparacao-memoria.csv). LaTeX: [ram.tex](../../tcc_2/tabelas/ram.tex).

## Periodicidade do LED em erro

| Placa | Ciclos | Média (ms) | Mín. (ms) | Máx. (ms) | Desvio (%) |
| --- | --- | --- | --- | --- | --- |
| ESP32 | 10 | 999,968 | 999,948 | 999,973 | -0,003% |
| nRF52840 | 45 | 1000,155 | 999,193 | 1000,968 | 0,015% |
| STM32F407 | 39 | 1003,897 | 1003,490 | 1004,286 | 0,390% |
| STM32F411 | 35 | 1030,123 | 1029,740 | 1030,395 | 3,012% |

Janela estável de ciclos completos durante falha do sensor; nominal de 1.000 ms. Desvio calculado sobre a média. Não mede diretamente as interrupções de 1 ms e não isola timer, laço, bloqueios ou aglutinação. As janelas, CSVs e valores sem arredondamento estão em led.csv; o roteiro não estabelece tolerância rígida. TIMER permanece LIMITADO nas quatro placas.

Fonte: [led.csv](led.csv). LaTeX: [led.tex](../../tcc_2/tabelas/led.tex).

## Conferência independente das capturas com framework

| Placa | Leituras I2C | Registros UART | Erros stop bit |
| --- | --- | --- | --- |
| ESP32 | 109 | 134 | 0 |
| nRF52840 | 80 | 127 | 0 |
| STM32F407 | 61 | 106 | 0 |
| STM32F411 | 40 | 77 | 0 |

Decodificação das transições digitais dos arquivos .sal a 2 MHz, preservada em conferencia-sal.json de cada sessão. Nas 290 leituras completas: comando 24 00, STOP antes da leitura separada de seis bytes, CRC e valores correspondentes à UART. Mensagens completas com CR LF. Nenhum erro de stop bit foi detectado com decodificador configurado a 115200 8N1; baud físico não medido. ESP32: três ERROR adicionais ausentes dos logs atuais permanecem na captura e fora da janela principal. Ver sinais.csv e LIMITACOES.csv.

Fonte: [sinais.csv](sinais.csv). LaTeX: [sinais.tex](../../tcc_2/tabelas/sinais.tex).
