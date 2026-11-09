# Sessão de bancada

- Criada: 2026-11-08T10:10:30-03:00
- Atualizada: 2026-11-08T21:32:10-03:00
- Responsável: fbcms
- Placa: nRF52840
- Versão: Com framework
- Projeto: nrf52840_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/nrf52840_framework/artefatos/zephyr/zephyr.elf
- SHA-256 ELF: 2d8b046629157a6ed126340b43b0878a032e83ec967eca85e793a35f3e895765
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

Conforme confirmação do responsável nesta revisão, o comportamento foi o mesmo da ESP32: ao pressionar o botão, a tela mudou para DIAGNOSTICO no momento do acionamento. Ao manter o botão pressionado, não houve mais trocas de tela nem outras ações. O botão funcionou normalmente. A duração elétrica do acionamento não foi medida nesta revisão.

### display_recuperacao

Conforme confirmação do responsável nesta revisão, o comportamento foi o mesmo da ESP32: após reconectar o módulo OLED, a tela de leituras voltou a ser exibida normalmente, sem reset do microcontrolador. A UART continuou recebendo amostras válidas.

### sensor_falha

O módulo SHT31-D foi removido inteiro. O log contém 23 registros ERROR;source=SHT31D;code=6 e não contém novas SAMPLE durante esse intervalo de erro. A foto mostra alerta ! e a última amostra preservada (22,2 °C e 57,5%), coerente com temperature_mC=22175;humidity_mpercent=57543 antes da falha. O display permaneceu operante.

### sensor_recuperacao

As amostras válidas retornaram às 10:13:19 no horário de recepção do computador, com 14 novas aquisições antes do primeiro erro do display. Esses horários não medem a latência física da reconexão.

### display_falha

O módulo OLED foi removido inteiro, mantendo o sensor conectado. O log contém 24 registros ERROR;source=SSD1306;code=5, com 12 SAMPLE válidas intercaladas durante o intervalo de erro do display.

### gravar

O log registrou o envio dos 181760 bytes do UF2 e a desconexão do volume durante a finalização. A execução posterior da aplicação está documentada pelas fotos, pelas amostras UART e pela sequência funcional confirmada pelo responsável.

### finalizar

Janela ativa do LED: digital.csv / ensaio_continuo.sal, entre as subidas em 92,6232105 s e 137,630182 s, com 45 ciclos completos e período médio de 1000,154922 ms. O recálculo confere com a tabela salva e a janela está contida no erro do sensor. TIMER permanece LIMITADO por ser evidência indireta. As aprovações físicas desses grupos são as informadas pelo responsável.

Conferência independente de I2C/UART em 08/11/2026: as transições digitais de `ensaio_continuo.sal` foram decodificadas por `tools/conferir_sal.py`. As 80 leituras completas do sensor passaram em comando 24 00, STOP antes da leitura separada, seis bytes, ACK/NACK, CRC e correspondência dos valores com SAMPLE. Foram decodificados 127 registros SAMPLE/ERROR completos com CR LF, sem erro de stop bit detectado a 115200 8N1. As transições do LED e o fim da captura coincidem exatamente com o CSV exportado. Detalhes, hashes e bytes selecionados em [conferencia-sal.json](conferencia-sal.json); tabela em [intervalos-sal.csv](intervalos-sal.csv).

| Trecho conferido | Protocolo | Início (s) | Fim (s) |
| --- | --- | --- | --- |
| Sensor em operação normal | I2C | 18.058 | 18.077 |
| Escrita normal no OLED | I2C | 18.080 | 18.095 |
| SAMPLE e CR LF na UART | UART | 18.093 | 18.099 |
| Falha principal do sensor | I2C/UART | 92.106 | 136.113 |
| Recuperação do sensor | I2C/UART | 138.108 | 140.150 |
| Falha do display | I2C/UART | 165.155 | 188.176 |
| Recuperação do display | I2C/UART | 189.171 | 192.191 |

Decodificação independente das transições digitais do .sal; não foi utilizada a interface gráfica do Logic 2. Janelas em segundos desde o início da captura, com pequena margem de localização; não medem latência, duração exata da mensagem ou baud físico. O NACK do mestre no último byte de uma leitura é normal e foi distinguido da falta de ACK do periférico. Falhas provocadas por remoção integral dos módulos, incluindo alimentação; observações visuais permanecem as confirmadas pelo responsável.


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-08T10:11:06-03:00
- Fim: 2026-11-08T10:14:42-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T10:11:11-03:00 | 5 | Inicie a captura no Saleae | 4 | 0 | 0 |
| 2026-11-08T10:11:19-03:00 | 13 | Pressione reset uma vez | 8 | 0 | 0 |
| 2026-11-08T10:11:44-03:00 | 38 | Observe três amostras e fotografe as leituras | 21 | 0 | 0 |
| 2026-11-08T10:11:59-03:00 | 53 | Abra o diagnóstico e tire a foto | 28 | 0 | 0 |
| 2026-11-08T10:12:04-03:00 | 58 | Volte à tela de leituras | 31 | 0 | 0 |
| 2026-11-08T10:12:20-03:00 | 74 | Mantenha o botão pressionado | 39 | 0 | 0 |
| 2026-11-08T10:12:25-03:00 | 79 | Termine o teste do botão na tela de leituras | 42 | 0 | 0 |
| 2026-11-08T10:12:45-03:00 | 99 | Remova o módulo do sensor | 45 | 6 | 0 |
| 2026-11-08T10:12:51-03:00 | 105 | Observe a falha do sensor e tire a foto | 45 | 9 | 0 |
| 2026-11-08T10:13:13-03:00 | 127 | Mantenha a falha e observe dez ciclos do LED | 45 | 21 | 0 |
| 2026-11-08T10:13:20-03:00 | 134 | Reconecte o sensor sem reset | 46 | 23 | 0 |
| 2026-11-08T10:13:27-03:00 | 141 | Confira a recuperação do sensor | 50 | 23 | 0 |
| 2026-11-08T10:13:50-03:00 | 164 | Remova o módulo do display | 61 | 23 | 4 |
| 2026-11-08T10:14:05-03:00 | 179 | Observe a falha do display e tire a foto | 69 | 23 | 20 |
| 2026-11-08T10:14:16-03:00 | 190 | Reconecte o display sem reset | 74 | 23 | 24 |
| 2026-11-08T10:14:24-03:00 | 198 | Confira a recuperação e as duas telas | 78 | 23 | 24 |
| 2026-11-08T10:14:42-03:00 | 216 | Pare e salve a captura completa | 87 | 23 | 24 |
## Gravações

- 2026-11-08T10:10:32-03:00: ENVIADO — gravacao_20261108_101032_502488.log

## UART

- serial_20261108_101102_186228.log; bytes originais: serial_20261108_101102_186228.bin; início: 2026-11-08T10:11:02-03:00; fim: 2026-11-08T10:14:42-03:00
  Estatísticas recebidas: {'samples': 87, 'sensor_errors': 23, 'display_errors': 24, 'bytes': 5951, 'last': 'SAMPLE;temperature_mC=22682;humidity_mpercent=53785'}

## Evidências

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
- preparar · Foto da montagem: 00_montagem_substituir.jpeg
- sensor_falha · CSV bruto do LED: digital.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_102813_212165.csv

## Arquivos esperados e cobertura

Nomes sugeridos: outros nomes e trechos complementares são aceitos. A conferência usa classificação e cobertura; o nome sozinho não comprova o conteúdo.
São cinco fotos básicas: montagem, leituras, diagnóstico, falha do sensor e falha do display. Uma captura contínua .sal, logs e observações registram inicialização, botão e recuperações. Imagens do analisador são opcionais e podem ser preparadas depois, no passo 7.

[OK] 00_montagem.jpg
Classificação: Foto da montagem. Cobertura: Montagem / preparação.
Fotografe a placa identificada, alimentação, jumpers e conexões dos dispositivos.
Na sessão: 00_montagem_substituir.jpeg

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
  "cycles": 45,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 500.0779222222213,
      "minimo": 499.07149999999945,
      "maximo": 500.7620000000088
    },
    "baixo_ms": {
      "media": 500.07700000000074,
      "minimo": 499.0645000000029,
      "maximo": 500.34399999999835
    },
    "periodo_ms": {
      "media": 1000.154922222222,
      "minimo": 999.1934999999899,
      "maximo": 1000.9675000000016
    }
  },
  "window_start_s": 92.6232105,
  "window_end_s": 137.630182,
  "selection_start_s": 92.123707,
  "selection_end_s": 137.630182,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_102813_212165.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 45.
Janela entre subidas: 92.623210500–137.630182000 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 500.077922 | 499.071500 | 500.762000 |
| Baixo | 500.077000 | 499.064500 | 500.344000 |
| Período | 1000.154922 | 999.193500 | 1000.967500 |

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
- Limitações: Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.

Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.
O ensaio direto HW-05 permanece não executado.

Resultados publicados nos CSVs em: 2026-11-08T21:32:10-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T21:32:10-03:00
- Situação: COMPLETA_COM_LIMITACOES
- Pendências: 0; limitações documentadas: 1
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

