# Sessão de bancada

- Criada: 2026-11-08T18:25:31-03:00
- Atualizada: 2026-11-08T18:31:45-03:00
- Responsável: fbcms
- Placa: STM32F411
- Versão: Sem framework
- Projeto: stm32f411_sem_framework
- Configuração: Debug (-Og)
- ELF: evidencias/firmwares/stm32f411_sem_framework/artefatos/stm32f411_sem_framework.elf
- SHA-256 ELF: a5a849702ef9e479c7041956d30308e94a20048fed054b80082acb436f320402
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
- Início: 2026-11-08T18:26:39-03:00
- Fim: 2026-11-08T18:28:25-03:00

Horários registrados pelo computador após a confirmação do responsável. O tempo decorrido não é o tempo exato do Saleae; use o reset e os sinais para correlacionar os eventos. Confirmar ações não aprova automaticamente os resultados físicos.

| Horário | Decorrido (s) | Ação confirmada | SAMPLE | Erros sensor | Erros display |
| --- | ---: | --- | ---: | ---: | ---: |
| 2026-11-08T18:26:45-03:00 | 6 | Pressione reset uma vez | 6 | 0 | 0 |
| 2026-11-08T18:26:57-03:00 | 18 | Observe três amostras e fotografe as leituras | 12 | 0 | 0 |
| 2026-11-08T18:27:08-03:00 | 29 | Abra o diagnóstico e tire a foto | 17 | 0 | 0 |
| 2026-11-08T18:27:09-03:00 | 30 | Volte à tela de leituras | 17 | 0 | 0 |
| 2026-11-08T18:27:16-03:00 | 37 | Mantenha o botão pressionado | 21 | 0 | 0 |
| 2026-11-08T18:27:19-03:00 | 40 | Termine o teste do botão na tela de leituras | 22 | 0 | 0 |
| 2026-11-08T18:27:22-03:00 | 43 | Remova o módulo do sensor | 23 | 1 | 0 |
| 2026-11-08T18:27:34-03:00 | 55 | Observe a falha do sensor e tire a foto | 23 | 7 | 0 |
| 2026-11-08T18:27:49-03:00 | 70 | Mantenha a falha e observe dez ciclos do LED | 23 | 14 | 0 |
| 2026-11-08T18:27:56-03:00 | 77 | Reconecte o sensor sem reset | 25 | 16 | 0 |
| 2026-11-08T18:28:03-03:00 | 84 | Confira a recuperação do sensor | 28 | 16 | 0 |
| 2026-11-08T18:28:06-03:00 | 87 | Remova o módulo do display | 29 | 16 | 0 |
| 2026-11-08T18:28:14-03:00 | 95 | Observe a falha do display e tire a foto | 33 | 16 | 8 |
| 2026-11-08T18:28:21-03:00 | 102 | Reconecte o display sem reset | 36 | 16 | 12 |
| 2026-11-08T18:28:23-03:00 | 104 | Confira a recuperação e as duas telas | 38 | 16 | 12 |
| 2026-11-08T18:28:25-03:00 | 106 | Conclua a observação contínua | 39 | 16 | 12 |
## Gravações

- 2026-11-08T18:26:10-03:00: ENVIADO — gravacao_20261108_182610_234808.log

## UART

- serial_20261108_182635_082194.log; bytes originais: serial_20261108_182635_082194.bin; início: 2026-11-08T18:26:35-03:00; fim: 2026-11-08T18:28:25-03:00
  Estatísticas recebidas: {'samples': 39, 'sensor_errors': 16, 'display_errors': 12, 'bytes': 2863, 'last': 'SAMPLE;temperature_mC=24797;humidity_mpercent=44974'}

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

Resultados publicados nos CSVs em: 2026-11-08T18:31:45-03:00


## Conferência de fechamento

- Conferida em: 2026-11-08T18:31:45-03:00
- Situação: COMPLETA
- Pendências: 0; limitações documentadas: 0
- Detalhes: verificacao-sessao.md e verificacao-sessao.json

