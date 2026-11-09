# Sessão de bancada

- Criada: 2026-11-08T17:08:08-03:00
- Atualizada: 2026-11-08T21:32:10-03:00
- Responsável: fbcms
- Placa: STM32F407
- Versão: Sem framework
- Projeto: stm32f407_sem_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/stm32f407_sem_framework/artefatos/stm32f407_sem_framework.elf
- SHA-256 ELF: 23661c09ac961be35878ca614ff56c4cb64bcd0e6866b73ce23271e0ba438eb7
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

O responsável informou nesta revisão que o pressionamento prolongado funcionou corretamente: houve uma troca ao acionar o botão, sem novas trocas enquanto permaneceu pressionado.

### sensor_falha

Na remoção do módulo SHT31-D, há 19 registros ERROR;source=SHT31D;code=6 entre 17:09:23 e 17:09:59, sem novas SAMPLE nesse intervalo. A foto mostra o sensor removido, alerta ! e última amostra preservada (23,6 °C e 51,0%), coerentes com temperature_mC=23582;humidity_mpercent=51006 antes da falha. O OLED permaneceu operante.

### sensor_recuperacao

As amostras retornam às 17:10:01 no horário de recepção do computador, com mais de duas aquisições válidas antes do primeiro erro do display.

### display_falha

O módulo OLED foi removido inteiro, com o sensor conectado. Antes de 17:10:38, a UART registra 26 erros do SSD1306 e 12 amostras válidas durante a falha do display. A ocorrência posterior de erros simultâneos dos dois periféricos está descrita na recuperação.

### display_recuperacao

O responsável informou nesta revisão que a recuperação visual do display foi correta, com retorno das telas após a reconexão. De 17:10:38.200351 a 17:10:56.631353, após a confirmação de reconexão do OLED às 17:10:38, aparecem dez erros do sensor e 19 erros do display, sem SAMPLE no intervalo. O responsável esclareceu que também mexeu nas conexões nesse momento. Portanto, esse trecho ocorreu durante manipulação adicional da montagem e não representa uma condição controlada de somente reconectar o OLED; não foi atribuído a falha espontânea do firmware. A última amostra anterior é às 17:10:36.197513; as amostras retornam às 17:10:58.433745, seguidas de mais duas válidas até o encerramento. A recuperação visual informada pelo responsável e os arquivos originais foram preservados.

### finalizar

Revisão: firmware e gravação verificados; texto e binário da UART coincidem (4873 bytes, 52 amostras, 29 erros do sensor e 45 erros do display). A janela complementar do LED foi corrigida para 51,7924975–88,946506 s, com 37 ciclos completos e período médio de 1004,162392 ms. O intervalo seguinte cruza a recuperação e continha somente 16,914 ms em nível baixo; foi excluído da medição estável, preservando-se a tabela anterior. A captura e a medição são opcionais nesta versão. Os erros simultâneos descritos na recuperação do display ocorreram durante manipulação adicional das conexões, confirmada pelo responsável. Essa intervenção fica documentada como limite da comparação desse trecho; a conferência automática verifica completude documental.

Conferência independente de I2C/UART em 08/11/2026: as transições digitais de `ensaio_continuo.sal` foram decodificadas por `tools/conferir_sal.py`. As 48 leituras completas do sensor passaram em comando 24 00, STOP antes da leitura separada, seis bytes, ACK/NACK, CRC e correspondência dos valores com SAMPLE. Foram decodificados 122 registros SAMPLE/ERROR completos com CR LF, sem erro de stop bit detectado a 115200 8N1. As transições do LED e o fim da captura coincidem exatamente com o CSV exportado. Detalhes, hashes e bytes selecionados em [conferencia-sal.json](conferencia-sal.json); tabela em [intervalos-sal.csv](intervalos-sal.csv).

| Trecho conferido | Protocolo | Início (s) | Fim (s) |
| --- | --- | --- | --- |
| Sensor em operação normal | I2C | 5.089 | 5.108 |
| Escrita normal no OLED | I2C | 5.108 | 5.121 |
| SAMPLE e CR LF na UART | UART | 5.119 | 5.126 |
| Falha principal do sensor | I2C/UART | 51.277 | 87.431 |
| Recuperação do sensor | I2C/UART | 89.435 | 91.480 |
| Falha do display | I2C/UART | 99.491 | 124.624 |
| Recuperação do display | I2C/UART | 145.181 | 147.958 |

Decodificação independente das transições digitais do .sal; não foi utilizada a interface gráfica do Logic 2. Janelas em segundos desde o início da captura, com pequena margem de localização; não medem latência, duração exata da mensagem ou baud físico. O NACK do mestre no último byte de uma leitura é normal e foi distinguido da falta de ACK do periférico. Falhas provocadas por remoção integral dos módulos, incluindo alimentação; observações visuais permanecem as confirmadas pelo responsável. Captura complementar, opcional nesta versão. Há diferenças de cobertura no início/fim entre a captura e o monitor UART; não foram tratadas como corrupção de mensagens. A janela da falha isolada do display termina em 124,624 s. O bloco posterior com erros conjuntos do sensor/display permanece associado à manipulação adicional das conexões confirmada pelo responsável. A recuperação registrada ocorre após esse bloco.


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-08T17:08:30-03:00
- Fim: 2026-11-08T17:11:03-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T17:08:37-03:00 | 7 | Pressione reset uma vez | 8 | 0 | 0 |
| 2026-11-08T17:08:50-03:00 | 20 | Observe três amostras e fotografe as leituras | 15 | 0 | 0 |
| 2026-11-08T17:09:00-03:00 | 30 | Abra o diagnóstico e tire a foto | 20 | 0 | 0 |
| 2026-11-08T17:09:05-03:00 | 35 | Volte à tela de leituras | 22 | 0 | 0 |
| 2026-11-08T17:09:15-03:00 | 45 | Mantenha o botão pressionado | 27 | 0 | 0 |
| 2026-11-08T17:09:18-03:00 | 48 | Termine o teste do botão na tela de leituras | 29 | 0 | 0 |
| 2026-11-08T17:09:24-03:00 | 54 | Remova o módulo do sensor | 31 | 1 | 0 |
| 2026-11-08T17:09:37-03:00 | 67 | Observe a falha do sensor e tire a foto | 31 | 7 | 0 |
| 2026-11-08T17:09:55-03:00 | 85 | Mantenha a falha e observe dez ciclos do LED | 31 | 16 | 0 |
| 2026-11-08T17:10:03-03:00 | 93 | Reconecte o sensor sem reset | 32 | 19 | 0 |
| 2026-11-08T17:10:07-03:00 | 97 | Confira a recuperação do sensor | 34 | 19 | 0 |
| 2026-11-08T17:10:12-03:00 | 102 | Remova o módulo do display | 37 | 19 | 1 |
| 2026-11-08T17:10:32-03:00 | 122 | Observe a falha do display e tire a foto | 47 | 19 | 21 |
| 2026-11-08T17:10:38-03:00 | 128 | Reconecte o display sem reset | 49 | 20 | 27 |
| 2026-11-08T17:11:01-03:00 | 151 | Confira a recuperação e as duas telas | 51 | 29 | 45 |
| 2026-11-08T17:11:03-03:00 | 153 | Conclua a observação contínua | 52 | 29 | 45 |
## Gravações

- 2026-11-08T17:08:13-03:00: ENVIADO — gravacao_20261108_170813_812176.log

## UART

- serial_20261108_170821_790692.log; bytes originais: serial_20261108_170821_790692.bin; início: 2026-11-08T17:08:21-03:00; fim: 2026-11-08T17:11:03-03:00
  Estatísticas recebidas: {'samples': 52, 'sensor_errors': 29, 'display_errors': 45, 'bytes': 4873, 'last': 'SAMPLE;temperature_mC=24557;humidity_mpercent=45180'}

## Evidências

- preparar · Foto da montagem: 00_montagem.jpeg
- normal · Imagem da tela de leituras: 01_tela_leituras.jpeg
- botao · Imagem da tela de diagnóstico: 02_tela_diagnostico.jpeg
- sensor_falha · Foto da falha do sensor: 03a_falha_sensor.jpeg
- display_falha · Foto da falha do display: 04a_falha_display.jpeg
- sensor_falha · CSV bruto do LED: digital.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_172828_371056.csv
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_173121_147006.csv

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
  "cycles": 37,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 502.07670270270296,
      "minimo": 501.8090000000086,
      "maximo": 502.42500000000234
    },
    "baixo_ms": {
      "media": 502.0856891891888,
      "minimo": 501.82250000000295,
      "maximo": 502.4530000000027
    },
    "periodo_ms": {
      "media": 1004.1623918918918,
      "minimo": 1003.7195000000025,
      "maximo": 1004.7840000000008
    }
  },
  "window_start_s": 51.7924975,
  "window_end_s": 88.946506,
  "selection_start_s": 51.7924975,
  "selection_end_s": 88.946506,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_173121_147006.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 37.
Janela entre subidas: 51.792497500–88.946506000 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 502.076703 | 501.809000 | 502.425000 |
| Baixo | 502.085689 | 501.822500 | 502.453000 |
| Período | 1004.162392 | 1003.719500 | 1004.784000 |
Resultados publicados nos CSVs em: 2026-11-08T17:32:12-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T21:32:10-03:00
- Situação: COMPLETA
- Pendências: 0; limitações documentadas: 0
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

