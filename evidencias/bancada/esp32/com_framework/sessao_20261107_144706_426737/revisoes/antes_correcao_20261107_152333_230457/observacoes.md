# Sessão de bancada

- Criada: 2026-11-07T14:47:06-03:00
- Atualizada: 2026-11-07T15:01:35-03:00
- Responsável: fbcms
- Placa: ESP32
- Versão: Com framework
- Projeto: esp32_framework
- Configuração: default (-Og)
- ELF: evidencias/firmwares/esp32_framework/artefatos/esp32_framework.elf
- SHA-256 ELF: f631e7dcdc06a54d924510d1573cf382ffd51a9e6f8b31911a18a67d113dcec9
- Identidade conferida nesta abertura: True

## Configuração

- port: /dev/ttyUSB0
- python: /home/fbcms/.espressif/tools/python/v6.1/venv/bin/python
- stm_tool: cube
- programmer: 
- volume: 
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

## Observações por cenário


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-07T14:47:37-03:00
- Fim: 2026-11-07T14:52:11-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-07T14:47:41-03:00 | 4 | Inicie a captura no Saleae | 7 | 0 | 0 |
| 2026-11-07T14:47:49-03:00 | 12 | Pressione reset uma vez | 12 | 0 | 0 |
| 2026-11-07T14:48:15-03:00 | 38 | Observe três amostras e fotografe as leituras | 25 | 0 | 0 |
| 2026-11-07T14:48:34-03:00 | 57 | Abra o diagnóstico e tire a foto | 34 | 0 | 0 |
| 2026-11-07T14:48:40-03:00 | 63 | Volte à tela de leituras | 38 | 0 | 0 |
| 2026-11-07T14:48:54-03:00 | 77 | Mantenha o botão pressionado | 44 | 0 | 0 |
| 2026-11-07T14:49:13-03:00 | 96 | Termine o teste do botão na tela de leituras | 54 | 0 | 0 |
| 2026-11-07T14:49:27-03:00 | 110 | Interrompa a comunicação do sensor | 61 | 0 | 0 |
| 2026-11-07T14:49:46-03:00 | 129 | Observe a falha do sensor e tire a foto | 67 | 3 | 0 |
| 2026-11-07T14:50:27-03:00 | 170 | Mantenha a falha e observe dez ciclos do LED | 82 | 9 | 0 |
| 2026-11-07T14:50:31-03:00 | 174 | Reconecte o sensor sem reset | 84 | 9 | 0 |
| 2026-11-07T14:50:37-03:00 | 180 | Confira a recuperação do sensor | 87 | 9 | 0 |
| 2026-11-07T14:50:45-03:00 | 188 | Interrompa a comunicação do display | 91 | 9 | 2 |
| 2026-11-07T14:51:02-03:00 | 205 | Observe a falha do display e tire a foto | 100 | 9 | 16 |
| 2026-11-07T14:51:05-03:00 | 208 | Reconecte o display sem reset | 101 | 9 | 16 |
| 2026-11-07T14:51:25-03:00 | 228 | Confira a recuperação e as duas telas | 111 | 9 | 16 |
| 2026-11-07T14:52:11-03:00 | 274 | Pare e salve a captura completa | 134 | 9 | 16 |
## Gravações

- 2026-11-07T14:47:12-03:00: ENVIADO — gravacao_20261107_144712_998911.log

## UART

- serial_20261107_144727_598811.log; bytes originais: serial_20261107_144727_598811.bin; início: 2026-11-07T14:47:27-03:00; fim: 2026-11-07T14:52:11-03:00
  Estatísticas recebidas: {'samples': 134, 'sensor_errors': 9, 'display_errors': 16, 'bytes': 9192, 'last': 'SAMPLE;temperature_mC=24461;humidity_mpercent=60955'}

## Evidências

- preparar · Foto da montagem: 00_montagem.jpeg
- normal · Imagem da tela de leituras: 01_tela_leituras.jpeg
- normal · Imagem da tela de leituras: 02_tela_diagnostico.jpeg
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
- sensor_falha · Tabela calculada do LED: led_intervalos_20261107_145949_558921.csv

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
Na sessão: 01_tela_leituras.jpeg, 02_tela_diagnostico.jpeg

[OK] 02_tela_diagnostico.jpg
Classificação: Imagem da tela de diagnóstico. Cobertura: Botão e telas.
Uma única foto de DIAGNOSTICO com última aquisição e tempo ativo legíveis. Descreva o resultado do botão mantido nas observações e marque sua duração no .sal; não são exigidas fotos antes/depois.
Na sessão: 02_tela_diagnostico.jpeg

[OK] 03a_falha_sensor.jpg
Classificação: Foto da falha do sensor. Cobertura: Falha do sensor.
Fotografe a tela de leituras com o alerta ! e a última amostra preservada durante a falha somente do sensor. Se possível, enquadre o jumper interrompido. A periodicidade do LED e a recuperação são registradas no .sal, na UART e nas observações.
Na sessão: 03a_falha_sensor.jpeg

[OK] 04a_falha_display.jpg
Classificação: Foto da falha do display. Cobertura: Falha do display.
Fotografe o ramo de comunicação do display interrompido, mantendo o OLED alimentado e o sensor conectado. A tela pode parecer normal ou estar congelada: relacione a foto aos erros UART, ao .sal e às observações. Não é necessária outra foto após a recuperação.
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
  "cycles": 37,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 4723.883972972975,
      "minimo": 130.5590000000052,
      "maximo": 103999.0185
    },
    "baixo_ms": {
      "media": 454.532229729728,
      "minimo": 12.907500000011396,
      "maximo": 500.0060000000133
    },
    "periodo_ms": {
      "media": 5178.416202702703,
      "minimo": 256.23349999997913,
      "maximo": 104498.929
    }
  },
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261107_145949_558921.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 37.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 4723.883973 | 130.559000 | 103999.018500 |
| Baixo | 454.532230 | 12.907500 | 500.006000 |
| Período | 5178.416203 | 256.233500 | 104498.929000 |

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

Resultados publicados nos CSVs em: 2026-11-07T15:01:26-03:00


## Conferência de fechamento

- Conferida em: 2026-11-07T15:01:26-03:00
- Situação: COMPLETA_COM_LIMITACOES
- Pendências: 0; limitações documentadas: 1
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

