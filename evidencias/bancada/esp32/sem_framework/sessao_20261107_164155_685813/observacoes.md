# Sessão de bancada

- Criada: 2026-11-07T16:41:55-03:00
- Atualizada: 2026-11-08T21:32:10-03:00
- Responsável: fbcms
- Placa: ESP32
- Versão: Sem framework
- Projeto: esp32_sem_framework
- Configuração: default (-Og)
- ELF: evidencias/firmwares/esp32_sem_framework/artefatos/esp32_sem_framework.elf
- SHA-256 ELF: 1cd5e6d8194b2b259115e78903ef325c9c113c985155db5d7d343ab2f9455561
- Identidade conferida nesta abertura: False

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

O responsável confirmou nesta revisão que o acionamento do botão provocou uma única troca de tela. Enquanto o botão permaneceu pressionado, não houve novas trocas. A navegação entre as telas permaneceu funcional.

### display_recuperacao

O responsável confirmou nesta revisão que, após reinserir o módulo OLED, as telas voltaram a ser exibidas normalmente, sem reiniciar a placa. As amostras UART continuaram sendo recebidas.

### finalizar

Conferência independente de I2C/UART em 08/11/2026: as transições digitais de `ensaio_continuo.sal` foram decodificadas por `tools/conferir_sal.py`. As 81 leituras completas do sensor passaram em comando 24 00, STOP antes da leitura separada, seis bytes, ACK/NACK, CRC e correspondência dos valores com SAMPLE. Foram decodificados 120 registros SAMPLE/ERROR completos com CR LF, sem erro de stop bit detectado a 115200 8N1. As transições do LED e o fim da captura coincidem exatamente com o CSV exportado. Detalhes, hashes e bytes selecionados em [conferencia-sal.json](conferencia-sal.json); tabela em [intervalos-sal.csv](intervalos-sal.csv).

| Trecho conferido | Protocolo | Início (s) | Fim (s) |
| --- | --- | --- | --- |
| Sensor em operação normal | I2C | 4.284 | 4.303 |
| Escrita normal no OLED | I2C | 4.303 | 4.317 |
| SAMPLE e CR LF na UART | UART | 4.315 | 4.322 |
| Falha principal do sensor | I2C/UART | 90.282 | 136.286 |
| Recuperação do sensor | I2C/UART | 138.281 | 140.319 |
| Falha do display | I2C/UART | 176.298 | 190.306 |
| Recuperação do display | I2C/UART | 191.302 | 194.317 |

Decodificação independente das transições digitais do .sal; não foi utilizada a interface gráfica do Logic 2. Janelas em segundos desde o início da captura, com pequena margem de localização; não medem latência, duração exata da mensagem ou baud físico. O NACK do mestre no último byte de uma leitura é normal e foi distinguido da falta de ACK do periférico. Falhas provocadas por remoção integral dos módulos, incluindo alimentação; observações visuais permanecem as confirmadas pelo responsável. Captura complementar, opcional nesta versão. Há diferenças de cobertura no início/fim entre a captura e o monitor UART; não foram tratadas como corrupção de mensagens. Transferência do OLED incompleta entre 1,822949 e 2,178138 s, coincidente com inicialização/reset: um bit pendente antes de novo START. As janelas normais foram selecionadas após essa inicialização; todas as 81 leituras completas do sensor passaram.


## Ensaio contínuo

- Situação: CONCLUIDO
- Início: 2026-11-07T16:42:19-03:00
- Fim: 2026-11-07T16:45:49-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-07T16:42:26-03:00 | 7 | Pressione reset uma vez | 6 | 0 | 0 |
| 2026-11-07T16:42:52-03:00 | 33 | Observe três amostras e fotografe as leituras | 19 | 0 | 0 |
| 2026-11-07T16:43:13-03:00 | 54 | Abra o diagnóstico e tire a foto | 30 | 0 | 0 |
| 2026-11-07T16:43:20-03:00 | 61 | Volte à tela de leituras | 33 | 0 | 0 |
| 2026-11-07T16:43:35-03:00 | 76 | Mantenha o botão pressionado | 41 | 0 | 0 |
| 2026-11-07T16:43:45-03:00 | 86 | Termine o teste do botão na tela de leituras | 46 | 0 | 0 |
| 2026-11-07T16:44:00-03:00 | 101 | Remova o módulo do sensor | 49 | 5 | 0 |
| 2026-11-07T16:44:11-03:00 | 112 | Observe a falha do sensor e tire a foto | 49 | 10 | 0 |
| 2026-11-07T16:44:32-03:00 | 133 | Mantenha a falha e observe dez ciclos do LED | 49 | 21 | 0 |
| 2026-11-07T16:44:42-03:00 | 143 | Reconecte o sensor sem reset | 50 | 24 | 0 |
| 2026-11-07T16:45:07-03:00 | 168 | Confira a recuperação do sensor | 63 | 24 | 0 |
| 2026-11-07T16:45:19-03:00 | 180 | Remova o módulo do display | 69 | 24 | 1 |
| 2026-11-07T16:45:28-03:00 | 189 | Observe a falha do display e tire a foto | 73 | 24 | 10 |
| 2026-11-07T16:45:35-03:00 | 196 | Reconecte o display sem reset | 77 | 24 | 15 |
| 2026-11-07T16:45:42-03:00 | 203 | Confira a recuperação e as duas telas | 81 | 24 | 15 |
| 2026-11-07T16:45:49-03:00 | 210 | Conclua a observação contínua | 84 | 24 | 15 |
## Gravações

- 2026-11-07T16:41:57-03:00: ENVIADO — gravacao_20261107_164157_562135.log

## UART

- serial_20261107_164215_743776.log; bytes originais: serial_20261107_164215_743776.bin; início: 2026-11-07T16:42:15-03:00; fim: 2026-11-07T16:45:49-03:00
  Estatísticas recebidas: {'samples': 84, 'sensor_errors': 24, 'display_errors': 15, 'bytes': 6251, 'last': 'SAMPLE;temperature_mC=26210;humidity_mpercent=61805'}

## Evidências

- preparar · Foto da montagem: 00_montagem.jpeg
- normal · Imagem da tela de leituras: 01_tela_leituras.jpeg
- botao · Imagem da tela de diagnóstico: 02_tela_diagnostico.jpeg
- sensor_falha · Foto da falha do sensor: 03a_falha_sensor.jpeg
- display_falha · Foto da falha do display: 04a_falha_display.jpeg

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

Resultados publicados nos CSVs em: 2026-11-07T16:57:44-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T21:32:10-03:00
- Situação: COMPLETA
- Pendências: 0; limitações documentadas: 0
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

