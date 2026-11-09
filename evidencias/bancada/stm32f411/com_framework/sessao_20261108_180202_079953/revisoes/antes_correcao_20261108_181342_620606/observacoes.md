# Sessão de bancada

- Criada: 2026-11-08T18:02:02-03:00
- Atualizada: 2026-11-08T18:12:21-03:00
- Responsável: fbcms
- Placa: STM32F411
- Versão: Com framework
- Projeto: stm32f411_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/stm32f411_framework/artefatos/stm32f411_framework.elf
- SHA-256 ELF: 67ac69d835b716d9a86dbea753ee7fb5ae28ea8967133b7532618fc1a3e4442d
- Identidade conferida nesta abertura: True

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


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-08T18:02:44-03:00
- Fim: 2026-11-08T18:04:56-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T18:02:57-03:00 | 13 | Inicie a captura no Saleae | 9 | 0 | 0 |
| 2026-11-08T18:03:00-03:00 | 16 | Pressione reset uma vez | 11 | 0 | 0 |
| 2026-11-08T18:03:12-03:00 | 28 | Observe três amostras e fotografe as leituras | 17 | 0 | 0 |
| 2026-11-08T18:03:21-03:00 | 37 | Abra o diagnóstico e tire a foto | 21 | 0 | 0 |
| 2026-11-08T18:03:23-03:00 | 39 | Volte à tela de leituras | 22 | 0 | 0 |
| 2026-11-08T18:03:31-03:00 | 47 | Mantenha o botão pressionado | 26 | 0 | 0 |
| 2026-11-08T18:03:34-03:00 | 50 | Termine o teste do botão na tela de leituras | 27 | 0 | 0 |
| 2026-11-08T18:03:39-03:00 | 55 | Remova o módulo do sensor | 29 | 1 | 0 |
| 2026-11-08T18:03:52-03:00 | 68 | Observe a falha do sensor e tire a foto | 29 | 7 | 0 |
| 2026-11-08T18:04:08-03:00 | 84 | Mantenha a falha e observe dez ciclos do LED | 29 | 15 | 0 |
| 2026-11-08T18:04:15-03:00 | 91 | Reconecte o sensor sem reset | 30 | 18 | 0 |
| 2026-11-08T18:04:21-03:00 | 97 | Confira a recuperação do sensor | 32 | 18 | 0 |
| 2026-11-08T18:04:26-03:00 | 102 | Remova o módulo do display | 35 | 18 | 1 |
| 2026-11-08T18:04:39-03:00 | 115 | Observe a falha do display e tire a foto | 41 | 18 | 13 |
| 2026-11-08T18:04:46-03:00 | 122 | Reconecte o display sem reset | 45 | 18 | 19 |
| 2026-11-08T18:04:52-03:00 | 128 | Confira a recuperação e as duas telas | 47 | 18 | 19 |
| 2026-11-08T18:04:56-03:00 | 132 | Pare e salve a captura completa | 49 | 18 | 19 |
## Gravações

- 2026-11-08T18:02:22-03:00: ENVIADO — gravacao_20261108_180222_125857.log

## UART

- serial_20261108_180238_513639.log; bytes originais: serial_20261108_180238_513639.bin; início: 2026-11-08T18:02:38-03:00; fim: 2026-11-08T18:04:56-03:00
  Estatísticas recebidas: {'samples': 49, 'sensor_errors': 18, 'display_errors': 19, 'bytes': 3652, 'last': 'SAMPLE;temperature_mC=24599;humidity_mpercent=44887'}

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
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_181001_534659.csv

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
  "cycles": 36,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 515.0666249999999,
      "minimo": 514.8580000000038,
      "maximo": 515.2669999999944
    },
    "baixo_ms": {
      "media": 501.22405555555576,
      "minimo": 17.076500000001715,
      "maximo": 515.2195000000006
    },
    "periodo_ms": {
      "media": 1016.2906805555557,
      "minimo": 532.1624999999983,
      "maximo": 1030.3954999999974
    }
  },
  "window_start_s": 43.411778,
  "window_end_s": 79.9982425,
  "selection_start_s": 42.8968515,
  "selection_end_s": 79.9982425,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_181001_534659.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 36.
Janela entre subidas: 43.411778000–79.998242500 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 515.066625 | 514.858000 | 515.267000 |
| Baixo | 501.224056 | 17.076500 | 515.219500 |
| Período | 1016.290681 | 532.162500 | 1030.395500 |

## Conformidade física e simulação complementar

### GPIO

- Caso: Níveis de LED/botão e relação com o comportamento observado.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: 

### I2C

- Caso: Endereços, comando, leitura separada, STOP e falha/recuperação.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: 

### UART

- Caso: Registros SAMPLE/ERROR e CRLF no fio, a 115200 8N1.
- Resultado físico: APROVADO_FISICO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: 

### TIMER

- Caso: Periodicidade do LED em erro: evidência indireta da base de tempo.
- Resultado físico: LIMITADO
- Simulação complementar: Consultar evidencias/simulacao/conformidade/MATRIZ.md.
- Limitações: Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.

Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.
O ensaio direto HW-05 permanece não executado.

Resultados publicados nos CSVs em: 2026-11-08T18:10:29-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T18:10:29-03:00
- Situação: COMPLETA_COM_LIMITACOES
- Pendências: 0; limitações documentadas: 1
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

