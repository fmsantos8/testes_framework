# Sessão de bancada

- Criada: 2026-11-08T15:10:09-03:00
- Atualizada: 2026-11-08T15:37:41-03:00
- Responsável: fbcms
- Placa: nRF52840
- Versão: Sem framework
- Projeto: nrf52840_sem_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/nrf52840_sem_framework/artefatos/zephyr/zephyr.elf
- SHA-256 ELF: 90c2a8823e0683e3017a120337d1b24132375bdec347fc3bfc6b235ef7751838
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
- Início: 2026-11-08T15:11:03-03:00
- Fim: 2026-11-08T15:14:04-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T15:11:24-03:00 | 21 | Pressione reset uma vez | 8 | 0 | 0 |
| 2026-11-08T15:11:50-03:00 | 47 | Observe três amostras e fotografe as leituras | 21 | 0 | 0 |
| 2026-11-08T15:12:15-03:00 | 72 | Abra o diagnóstico e tire a foto | 34 | 0 | 0 |
| 2026-11-08T15:12:21-03:00 | 78 | Volte à tela de leituras | 37 | 0 | 0 |
| 2026-11-08T15:12:29-03:00 | 86 | Mantenha o botão pressionado | 41 | 0 | 0 |
| 2026-11-08T15:12:35-03:00 | 92 | Termine o teste do botão na tela de leituras | 43 | 0 | 0 |
| 2026-11-08T15:12:40-03:00 | 97 | Remova o módulo do sensor | 45 | 1 | 0 |
| 2026-11-08T15:12:59-03:00 | 116 | Observe a falha do sensor e tire a foto | 45 | 11 | 0 |
| 2026-11-08T15:13:15-03:00 | 132 | Mantenha a falha e observe dez ciclos do LED | 45 | 19 | 0 |
| 2026-11-08T15:13:20-03:00 | 137 | Reconecte o sensor sem reset | 46 | 20 | 0 |
| 2026-11-08T15:13:29-03:00 | 146 | Confira a recuperação do sensor | 51 | 20 | 0 |
| 2026-11-08T15:13:36-03:00 | 153 | Remova o módulo do display | 54 | 20 | 2 |
| 2026-11-08T15:13:49-03:00 | 166 | Observe a falha do display e tire a foto | 61 | 20 | 15 |
| 2026-11-08T15:13:54-03:00 | 171 | Reconecte o display sem reset | 63 | 20 | 18 |
| 2026-11-08T15:14:01-03:00 | 178 | Confira a recuperação e as duas telas | 67 | 20 | 18 |
| 2026-11-08T15:14:04-03:00 | 181 | Conclua a observação contínua | 68 | 20 | 18 |
## Gravações

- 2026-11-08T15:10:45-03:00: ENVIADO — gravacao_20261108_151045_609259.log

## UART

- serial_20261108_151059_885089.log; bytes originais: serial_20261108_151059_885089.bin; início: 2026-11-08T15:10:59-03:00; fim: 2026-11-08T15:14:04-03:00
  Estatísticas recebidas: {'samples': 68, 'sensor_errors': 20, 'display_errors': 18, 'bytes': 4686, 'last': 'SAMPLE;temperature_mC=22132;humidity_mpercent=52119'}

## Evidências

- preparar · Foto da montagem: 00_montagem.jpeg
- normal · Imagem da tela de leituras: 01_tela_leituras.jpeg
- botao · Imagem da tela de diagnóstico: 02_tela_diagnostico.jpeg
- sensor_falha · Foto da falha do sensor: 03a_falha_sensor.jpeg
- display_falha · Foto da falha do display: 04a_falha_display.jpeg
- sensor_falha · CSV bruto do LED: digital.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_152612_219381.csv

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
      "media": 500.0779358974355,
      "minimo": 499.05749999999216,
      "maximo": 500.76000000001386
    },
    "baixo_ms": {
      "media": 500.1155256410262,
      "minimo": 498.9995000000107,
      "maximo": 500.3655000000009
    },
    "periodo_ms": {
      "media": 1000.1934615384617,
      "minimo": 999.220500000007,
      "maximo": 1000.9594999999933
    }
  },
  "window_start_s": 103.029489,
  "window_end_s": 142.037034,
  "selection_start_s": 102.5298965,
  "selection_end_s": 142.037034,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_152612_219381.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 39.
Janela entre subidas: 103.029489000–142.037034000 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 500.077936 | 499.057500 | 500.760000 |
| Baixo | 500.115526 | 498.999500 | 500.365500 |
| Período | 1000.193462 | 999.220500 | 1000.959500 |
Resultados publicados nos CSVs em: 2026-11-08T15:26:46-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T15:26:46-03:00
- Situação: COMPLETA
- Pendências: 0; limitações documentadas: 0
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

