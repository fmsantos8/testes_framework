# Sessão de bancada

- Criada: 2026-11-08T18:02:02-03:00
- Atualizada: 2026-11-08T21:32:10-03:00
- Responsável: fbcms
- Placa: STM32F411
- Versão: Com framework
- Projeto: stm32f411_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/stm32f411_framework/artefatos/stm32f411_framework.elf
- SHA-256 ELF: 67ac69d835b716d9a86dbea753ee7fb5ae28ea8967133b7532618fc1a3e4442d
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

O responsável confirmou nesta revisão que o acionamento prolongado funcionou perfeitamente: houve uma troca de tela ao pressionar, sem novas trocas enquanto o botão permaneceu pressionado. A duração elétrica não foi medida nesta revisão.

### sensor_falha

O módulo SHT31-D foi removido inteiro. A UART contém 18 registros ERROR;source=SHT31D;code=6 entre 18:03:38 e 18:04:13, sem novas SAMPLE nesse intervalo. A foto mostra o sensor removido, OLED operante, alerta ! e a última amostra preservada (23,6 °C e 48,9%), coerente com temperature_mC=23638;humidity_mpercent=48920 antes da falha.

### sensor_recuperacao

As amostras retornam às 18:04:15 no horário de recepção do computador, com mais de duas aquisições válidas antes do primeiro erro do display. Esses horários não medem exatamente a latência física da reconexão.

### display_falha

O OLED foi removido inteiro, mantendo o sensor conectado. Entre o primeiro e o último erro do display há 19 registros ERROR;source=SSD1306;code=5 e nove SAMPLE válidas intercaladas, sem erros do sensor nesse trecho.

### display_recuperacao

O responsável confirmou nesta revisão que o display funcionou perfeitamente e voltou a exibir normalmente as telas após a reconexão, sem reset do microcontrolador. A UART registra cinco amostras válidas após o último erro do display até o encerramento.

### finalizar

Firmware, gravação e arquivos UART conferidos: texto e binário coincidem, com 3652 bytes, 49 amostras, 18 erros do sensor e 19 erros do display. A janela do LED foi corrigida para 43,411778–79,46608 s, preservando 35 ciclos completos consecutivos. O intervalo seguinte, até 79,9982425 s, tinha apenas 17,0765 ms em nível baixo e cruza a saída do erro; foi excluído da medição estável, mantendo-se a tabela anterior. O período médio corrigido é 1030,122914 ms, cerca de 3,012% acima do nominal de 1 s. A medição indireta não isola a causa dessa diferença. As aprovações físicas desses grupos permanecem as informadas pelo responsável.

Conferência independente de I2C/UART em 08/11/2026: as transições digitais de `ensaio_continuo.sal` foram decodificadas por `tools/conferir_sal.py`. As 40 leituras completas do sensor passaram em comando 24 00, STOP antes da leitura separada, seis bytes, ACK/NACK, CRC e correspondência dos valores com SAMPLE. Foram decodificados 77 registros SAMPLE/ERROR completos com CR LF, sem erro de stop bit detectado a 115200 8N1. As transições do LED e o fim da captura coincidem exatamente com o CSV exportado. Detalhes, hashes e bytes selecionados em [conferencia-sal.json](conferencia-sal.json); tabela em [intervalos-sal.csv](intervalos-sal.csv).

| Trecho conferido | Protocolo | Início (s) | Fim (s) |
| --- | --- | --- | --- |
| Sensor em operação normal | I2C | 5.774 | 5.793 |
| Escrita normal no OLED | I2C | 5.793 | 5.807 |
| SAMPLE e CR LF na UART | UART | 5.805 | 5.812 |
| Falha principal do sensor | I2C/UART | 42.882 | 77.910 |
| Recuperação do sensor | I2C/UART | 79.966 | 82.065 |
| Falha do display | I2C/UART | 90.283 | 108.823 |
| Recuperação do display | I2C/UART | 109.849 | 112.954 |

Decodificação independente das transições digitais do .sal; não foi utilizada a interface gráfica do Logic 2. Janelas em segundos desde o início da captura, com pequena margem de localização; não medem latência, duração exata da mensagem ou baud físico. O NACK do mestre no último byte de uma leitura é normal e foi distinguido da falta de ACK do periférico. Falhas provocadas por remoção integral dos módulos, incluindo alimentação; observações visuais permanecem as confirmadas pelo responsável.


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
- sensor_falha · Tabela calculada do LED: led_intervalos_20261108_181342_664947.csv

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
  "cycles": 35,
  "at_least_ten": true,
  "unit": "ms",
  "statistics": {
    "alto_ms": {
      "media": 515.0660714285715,
      "minimo": 514.8580000000038,
      "maximo": 515.2669999999944
    },
    "baixo_ms": {
      "media": 515.056842857143,
      "minimo": 514.868000000007,
      "maximo": 515.2195000000006
    },
    "periodo_ms": {
      "media": 1030.1229142857144,
      "minimo": 1029.7399999999968,
      "maximo": 1030.3954999999974
    }
  },
  "window_start_s": 43.411778,
  "window_end_s": 79.46608,
  "selection_start_s": 43.411778,
  "selection_end_s": 79.46608,
  "limitation": "Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.",
  "method": "csv",
  "table": "led_intervalos_20261108_181342_664947.csv"
}
~~~

Tempos calculados a partir das transições do CSV do LED.
Ciclos: 35.
Janela entre subidas: 43.411778000–79.466080000 s.

| Medida (ms) | Média | Mínimo | Máximo |
| --- | --- | --- | --- |
| Alto | 515.066071 | 514.858000 | 515.267000 |
| Baixo | 515.056843 | 514.868000 | 515.219500 |
| Período | 1030.122914 | 1029.740000 | 1030.395500 |

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
- Limitações: Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações. Período médio observado do LED: 1030,122914 ms em 35 ciclos estáveis, cerca de 3,012% acima do nominal. Essa diferença não foi atribuída ao timer isoladamente; a medição inclui o comportamento da aplicação e o roteiro não define tolerância rígida.

Período de 1 ms das interrupções não medido diretamente. O LED incorpora atrasos do laço e possíveis perdas/aglutinações.
O ensaio direto HW-05 permanece não executado.

Resultados publicados nos CSVs em: 2026-11-08T21:32:10-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T21:32:10-03:00
- Situação: COMPLETA_COM_LIMITACOES
- Pendências: 0; limitações documentadas: 1
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

