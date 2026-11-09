# Sessão de bancada

- Criada: 2026-11-08T16:46:13-03:00
- Atualizada: 2026-11-08T21:32:10-03:00
- Responsável: fbcms
- Placa: STM32F407
- Versão: Com framework
- Projeto: stm32f407_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/stm32f407_framework/artefatos/stm32f407_framework.elf
- SHA-256 ELF: 1129efdd60735e767d37f81e426caebb41a83347d54fcf0820a9a64af02b2538
- Identidade conferida nesta abertura: False

## Configuração

- port: /dev/ttyUSB0
- python: /home/fbcms/.espressif/tools/python/v6.1/venv/bin/python
- stm_tool: cube
- programmer: 
- volume: /run/media/fbcms/NICENANO
- baud: 460800
- operator: fbcms
- saleae: 
- led_column: 

## Configuração fixa do Saleae

- Modelo do analisador: Analyzer 24MHz8CH
- Versão do Saleae Logic 2: 2.4.48
- Taxa de amostragem digital (Hz): 2000000
- Correspondência canais → sinais: 0=SCL, 1=SDA, 2=UART_TX, 3=LED, 4=BOTÃO

## Resultados funcionais

| Cenário | Resultado |
| --- | --- |
| Inicialização | APROVADO |
| Leituras normais | APROVADO |
| Botão e telas | APROVADO |
| UART | APROVADO |
| Falha do sensor | APROVADO |
| Recuperação do sensor | APROVADO |
| Falha do display | APROVADO |
| Recuperação do display | APROVADO |

## Método de injeção de falha

Remoção completa e reinserção de cada módulo, interrompendo sua alimentação e comunicação. O outro periférico permanece conectado; o microcontrolador não é reiniciado durante a recuperação.

## Observações por cenário

### botao

O responsável confirmou nesta revisão que, nesta execução do STM32F407 com framework, o botão provocou uma única troca de tela ao ser acionado. Enquanto permaneceu pressionado, não houve novas trocas. A navegação entre as telas funcionou normalmente. A duração elétrica não foi medida nesta revisão.

### sensor_falha

O módulo SHT31-D foi removido inteiro. O log contém 20 registros ERROR;source=SHT31D;code=6, sem novas SAMPLE durante o intervalo de erro do sensor. A foto mostra o módulo removido, alerta ! e última amostra preservada (23,2 °C e 50,6%), coerentes com temperature_mC=23227;humidity_mpercent=50608 antes da falha. O OLED permaneceu operante.

### sensor_recuperacao

As amostras retornam às 16:49:01 no horário de recepção do computador. Há mais de duas aquisições válidas antes dos erros do display; esses horários não medem a latência física exata da reconexão.

### display_falha

O módulo OLED foi removido inteiro e o sensor permaneceu conectado. Há 25 registros ERROR;source=SSD1306;code=5 e 12 amostras válidas intercaladas entre o primeiro e o último erro do display.

### display_recuperacao

O responsável confirmou nesta revisão que, após reinserir o OLED, as telas voltaram a ser exibidas normalmente, sem reset do microcontrolador. As amostras UART continuaram sendo recebidas.

### finalizar

A janela do LED foi corrigida para 73,571313–112,7232845 s em digital.csv / ensaio_continuo.sal, preservando os 39 ciclos completos consecutivos de erro. O intervalo seguinte, que terminava em 113,242365 s e tinha apenas 16,958 ms em nível baixo, corresponde ao retorno à operação normal e foi excluído da medição estável. O período médio corrigido é 1003,896705 ms. As aprovações físicas desses grupos são as informadas pelo responsável.

Conferência independente de I2C/UART em 08/11/2026: as transições digitais de `ensaio_continuo.sal` foram decodificadas por `tools/conferir_sal.py`. As 61 leituras completas do sensor passaram em comando 24 00, STOP antes da leitura separada, seis bytes, ACK/NACK, CRC e correspondência dos valores com SAMPLE. Foram decodificados 106 registros SAMPLE/ERROR completos com CR LF, sem erro de stop bit detectado a 115200 8N1. As transições do LED e o fim da captura coincidem exatamente com o CSV exportado. Detalhes, hashes e bytes selecionados em [conferencia-sal.json](conferencia-sal.json); tabela em [intervalos-sal.csv](intervalos-sal.csv).

| Trecho conferido | Protocolo | Início (s) | Fim (s) |
| --- | --- | --- | --- |
| Sensor em operação normal | I2C | 8.769 | 8.788 |
| Escrita normal no OLED | I2C | 8.787 | 8.801 |
| SAMPLE e CR LF na UART | UART | 8.799 | 8.806 |
| Falha principal do sensor | I2C/UART | 73.056 | 111.208 |
| Recuperação do sensor | I2C/UART | 113.212 | 115.256 |
| Falha do display | I2C/UART | 123.269 | 147.374 |
| Recuperação do display | I2C/UART | 148.374 | 151.401 |

Decodificação independente das transições digitais do .sal; não foi utilizada a interface gráfica do Logic 2. Janelas em segundos desde o início da captura, com pequena margem de localização; não medem latência, duração exata da mensagem ou baud físico. O NACK do mestre no último byte de uma leitura é normal e foi distinguido da falta de ACK do periférico. Falhas provocadas por remoção integral dos módulos, incluindo alimentação; observações visuais permanecem as confirmadas pelo responsável.


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-08T16:46:50-03:00
- Fim: 2026-11-08T16:50:10-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T16:47:08-03:00 | 18 | Inicie a captura no Saleae | 14 | 0 | 0 |
| 2026-11-08T16:47:15-03:00 | 25 | Pressione reset uma vez | 18 | 0 | 0 |
| 2026-11-08T16:47:41-03:00 | 51 | Observe três amostras e fotografe as leituras | 31 | 0 | 0 |
| 2026-11-08T16:47:47-03:00 | 57 | Abra o diagnóstico e tire a foto | 34 | 0 | 0 |
| 2026-11-08T16:47:56-03:00 | 66 | Volte à tela de leituras | 39 | 0 | 0 |
| 2026-11-08T16:48:08-03:00 | 78 | Mantenha o botão pressionado | 44 | 0 | 0 |
| 2026-11-08T16:48:12-03:00 | 82 | Termine o teste do botão na tela de leituras | 46 | 0 | 0 |
| 2026-11-08T16:48:23-03:00 | 93 | Remova o módulo do sensor | 50 | 2 | 0 |
| 2026-11-08T16:48:39-03:00 | 109 | Observe a falha do sensor e tire a foto | 50 | 10 | 0 |
| 2026-11-08T16:48:56-03:00 | 126 | Mantenha a falha e observe dez ciclos do LED | 50 | 18 | 0 |
| 2026-11-08T16:49:02-03:00 | 132 | Reconecte o sensor sem reset | 51 | 20 | 0 |
| 2026-11-08T16:49:06-03:00 | 136 | Confira a recuperação do sensor | 53 | 20 | 0 |
| 2026-11-08T16:49:16-03:00 | 146 | Remova o módulo do display | 58 | 20 | 6 |
| 2026-11-08T16:49:30-03:00 | 160 | Observe a falha do display e tire a foto | 65 | 20 | 20 |
| 2026-11-08T16:49:37-03:00 | 167 | Reconecte o display sem reset | 69 | 20 | 25 |
| 2026-11-08T16:49:45-03:00 | 175 | Confira a recuperação e as duas telas | 73 | 20 | 25 |
| 2026-11-08T16:50:10-03:00 | 200 | Pare e salve a captura completa | 85 | 20 | 25 |
## Gravações

- 2026-11-08T16:46:27-03:00: ENVIADO — gravacao_20261108_164627_034837.log

## UART

- serial_20261108_164640_401388.log; bytes originais: serial_20261108_164640_401388.bin; início: 2026-11-08T16:46:40-03:00; fim: 2026-11-08T16:50:10-03:00
  Estatísticas recebidas: {'samples': 85, 'sensor_errors': 20, 'display_errors': 25, 'bytes': 5790, 'last': 'SAMPLE;temperature_mC=23876;humidity_mpercent=46581'}

## Evidências

- preparar · Foto da montagem: 00_montagem.jpeg
- normal · Imagem da tela de leituras: 01_tela_leituras.jpeg
- botao · Imagem da tela de diagnóstico: 02_tela_diagnostico.jpeg
- sensor_falha · Foto da falha do sensor: 03a_falha_sensor.jpeg
- display_falha · Foto da falha do display: 04a_falha_display.jpeg
- inicio · Captura Saleae (.sal): ensaio_continuo.sal
- normal · Captura Saleae (.sal): ensaio_continuo.sal
- botao · Captura Saleae (.sal): ensaio_continuo.sal
- sensor_falha · Captura Saleae (.sal): ensaio_continuo.sal
- sensor_recuperacao · Captura Saleae (.sal): ensaio_continuo.sal
- display_falha · Captura Saleae (.sal): ensaio_continuo.sal
- display_recuperacao · Captura Saleae (.sal): ensaio_continuo.sal
- sensor_falha · CSV bruto do LED: digital.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_170048_453032.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_170402_029400.csv

## Arquivos esperados e cobertura

Nomes sugeridos: outros nomes e trechos complementares são aceitos. A conferência usa classificação e cobertura; o nome sozinho não comprova o conteúdo.
São cinco fotos básicas: montagem, leituras, diagnóstico, falha do sensor e falha do display. Uma captura contínua .sal, logs e observações registram inicialização, botão e recuperações. Imagens do analisador são opcionais e podem ser preparadas depois, no passo 7.

[OK] 00_montagem.jpg
Classificação: Foto da montagem. Cobertura: Montagem / preparação.
Fotografe a placa identificada, alimentação, jumpers e conexões dos dispositivos.
Na sessão: 00_montagem.jpeg

[OK] 01_tela_leituras.jpg
Classificação: Imagem da tela de leituras. Cobertura: Leituras normais.
Uma única foto legível de temperatura e umidade documenta a aparência da tela inicial e a operação normal. Registre o reset e a inicialização nas observações, na UART e no .sal; não é necessária outra foto igual.
Na sessão: 01_tela_leituras.jpeg

[OK] 02_tela_diagnostico.jpg
Classificação: Imagem da tela de diagnóstico. Cobertura: Botão e telas.
Uma única foto de DIAGNOSTICO com última aquisição e tempo ativo legíveis. Descreva o resultado do botão mantido nas observações e marque sua duração no .sal; não são exigidas fotos antes/depois.
Na sessão: 02_tela_diagnostico.jpeg

[OK] 03a_falha_sensor.jpg
Classificação: Foto da falha do sensor. Cobertura: Falha do sensor.
Fotografe a tela de leituras com o alerta ! e a última amostra preservada durante a falha somente do sensor. Se possível, enquadre o conector sem o módulo do sensor. A periodicidade do LED e a recuperação são registradas no .sal, na UART e nas observações.
Na sessão: 03a_falha_sensor.jpeg

[OK] 04a_falha_display.jpg
Classificação: Foto da falha do display. Cobertura: Falha do display.
Fotografe o conector sem o módulo OLED, mantendo o sensor conectado. A remoção completa interrompe alimentação e comunicação: relacione a foto aos erros UART, ao .sal e às observações. Não é necessária outra foto após a recuperação.
Na sessão: 04a_falha_display.jpeg

[OK] ensaio_continuo.sal
Classificação: Captura Saleae (.sal). Cobertura: Inicialização, Leituras normais, Botão e telas, Falha do sensor, Recuperação do sensor, Falha do display, Recuperação do display.
Inicie antes do reset e mantenha uma única captura durante leituras, botão, falhas e recuperações. Pare somente após recuperar o display. Anexe o arquivo uma vez para criar todas as coberturas. Capturas antigas separadas continuam aceitas quando associadas somente aos cenários que registraram.
Na sessão: ensaio_continuo.sal

Medições do LED
Use Medir LED no CSV para calcular os tempos a partir da janela estável em erro exportada do Logic 2 (tempo em segundos, canal LED). Ou use Registrar medições do LED para informar janela, ciclos e média/mínimo/máximo de alto, baixo e período. A fonte é a captura .sal da falha do sensor; imagem de marcadores é opcional.

Arquivos automáticos
Logs de gravação/UART e relatórios são automáticos. Verificar sessão também confere esses arquivos e os resultados.


## Periodicidade do LED

Evidência indireta; janela registrada sob erro. O método e as fontes constam abaixo.
~~~json
{
  "source": "digital.csv",
  "column": "LED",
  "cycles": 39,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 501.95707692307826,
      "minimo": 501.68499999999483,
      "maximo": 502.17900000001237
    },
    "baixo_ms": {
      "media": 501.93962820512695,
      "minimo": 501.6999999999996,
      "maximo": 502.1265000000028
    },
    "periodo_ms": {
      "media": 1003.8967051282052,
      "minimo": 1003.4904999999981,
      "maximo": 1004.2855000000088
    }
  },
  "window_start_s": 73.571313,
  "window_end_s": 112.7232845,
  "selection_start_s": 73.571313,
  "selection_end_s": 112.7232845,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_170402_029400.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 39.
Janela entre subidas: 73.571313000–112.723284500 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 501.957077 | 501.685000 | 502.179000 |
| Baixo | 501.939628 | 501.700000 | 502.126500 |
| Período | 1003.896705 | 1003.490500 | 1004.285500 |

## Conformidade física e simulação complementar

### GPIO

- Caso: Níveis de LED/botão e relação com o comportamento observado.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: 

### I2C

- Caso: Decodificação do .sal: endereços 0x44/0x3C, comando 24 00, STOP, leitura separada de seis bytes com CRC válido e falha/recuperação por remoção dos módulos. Intervalos em observacoes.md e conferencia-sal.json.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: A injeção de falha inclui perda de alimentação e comunicação do módulo removido; não isola uma interrupção somente nos sinais de comunicação.

### UART

- Caso: Decodificação independente do fio a 115200 8N1: SAMPLE/ERROR, CR LF e valores correspondentes às leituras I2C. Intervalos em observacoes.md e conferencia-sal.json.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: Decodificação configurada a 115200 8N1; baud físico não medido.

### TIMER

- Caso: Periodicidade do LED em erro: evidência indireta da base de tempo.
- Resultado físico: LIMITADO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações. Período médio observado do LED: 1003,896705 ms na janela estável de 39 ciclos; a medição inclui o comportamento da aplicação e não isola a causa da diferença em relação ao nominal.

Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.
O ensaio direto HW-05 permanece não executado.

Resultados publicados nos CSVs em: 2026-11-08T21:32:10-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T21:32:10-03:00
- Situação: COMPLETA_COM_LIMITACOES
- Pendências: 0; limitações documentadas: 1
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

