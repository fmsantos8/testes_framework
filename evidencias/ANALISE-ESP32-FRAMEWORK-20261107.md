# Análise da sessão ESP32 com framework para o passo 7

Análise em 07/11/2026 da sessão `bancada/esp32/com_framework/sessao_20261107_144706_426737/`, comparada ao passo 7 de `ROTEIRO.md`. Inclui a orientação do responsável nesta conversa: a campanha usará **remoção completa e reinserção de cada módulo**, pois a montagem não permite interromper somente seu ramo de comunicação.

O formato de coleta é aproveitável para o artigo. A revisão solicitada pelo responsável corrigiu a janela do LED, registrou o comportamento do botão mantido e o retorno visual do OLED, e alinhou roteiro e software à remoção completa dos módulos. A sessão foi conferida e republicada com zero pendências automáticas e TIMER limitado. Ainda falta transcrever os intervalos específicos usados nas conferências de I2C/UART. O estado automático `COMPLETA_COM_LIMITACOES` verifica cobertura documental; não certifica a interpretação experimental.

Esta análise acompanha a revisão de `sessao.json` e `observacoes.md`. O CSV bruto, o `.sal`, os logs UART e a tabela anterior do LED foram preservados. Os registros anteriores à correção estão em `revisoes/antes_correcao_20261107_152333_230457/`; a republicação também gerou backup dos CSVs centrais.

## Método de injeção de falha adotado

O responsável confirmou que retirou cada módulo inteiro. Isso corresponde às fotos: o SHT31-D está ausente em `03a_falha_sensor.jpeg`; o OLED está ausente em `04a_falha_display.jpeg`.

A descrição do método pode ser:

> A indisponibilidade de cada periférico foi provocada pela remoção completa de seu módulo do conector, interrompendo alimentação e comunicação. O outro periférico permaneceu conectado. Foram observados os erros reportados pela aplicação, a continuidade das funções disponíveis e o comportamento após a reinserção, sem reinicialização intencional do microcontrolador.

A remoção é um método de provocar falha de comunicação observável pela aplicação. Ela também inclui perda de alimentação do módulo; portanto, a conclusão deve se referir a **remoção/reinserção ou indisponibilidade do periférico**, sem alegar isolamento experimental de uma falha exclusivamente nos sinais de comunicação.

Essa escolha não exige repetir a sessão por si só. O roteiro, as ações do ensaio contínuo, o guia e as instruções dos anexos no software foram atualizados. O responsável confirmou que, após reconectar o OLED, a tela de leituras voltou a ser exibida normalmente; essa observação foi registrada na recuperação do display. O desaparecimento de `ERROR;source=SSD1306` sozinho não comprova recuperação visual após a perda de alimentação.

## Material que já funciona

| Material | Avaliação |
| --- | --- |
| Identificação do firmware | Os sete artefatos registrados em `sessao.json` conferem com seus SHA-256 atuais. |
| Gravação | Log registra envio concluído e verificação dos dados pelo esptool. |
| UART | Há 134 `SAMPLE` e 16 erros SSD1306 nos dois arquivos. O texto atual contém nove erros SHT31-D; o binário atual contém seis. Ambos foram preservados, com a divergência documentada abaixo. |
| Fotos | As cinco condições possuem fotos distintas. Leituras, diagnóstico e alerta de falha do sensor estão legíveis. |
| Captura Saleae | Arquivo `.sal` contém os canais e configurações dos analisadores I2C e Async Serial. Metadados registram 2 MHz e UART 115200 8N1. |
| Resultados centrais | Os oito campos funcionais da ESP32 com framework foram publicados como aprovados; GPIO/I2C/UART como aprovação física e TIMER como limitado. Esses estados são declarações do responsável. |
| Métricas | `metricas/RESUMO.md` já fornece as comparações de código, dependências e memória do passo 6. |

As fotos normais são adequadas para selecionar e recortar figuras do artigo. A foto da falha do sensor mostra `!`, 24,5 °C e 65,1%; esses valores são coerentes com a amostra válida `temperature_mC=24474;humidity_mpercent=65148` preservada no log.

## O que a sequência UART sustenta

Os horários abaixo são de recepção no computador, não medidas de latência física da intervenção.

| Intervalo/evento | Evidência encontrada |
| --- | --- |
| Reset inicial do ensaio, 14:47:48 | Registro de boot, seguido de amostras válidas. O boot anterior, 14:47:27, precede esse reset planejado. Não há outra mensagem de boot posteriormente no log. |
| Indisponibilidade do sensor | Registros `ERROR;source=SHT31D;code=6` e retomada das amostras válidas. |
| Indisponibilidade do display, 14:50:44–14:50:59 | Dezesseis registros `ERROR;source=SSD1306;code=5`, com amostras válidas intercaladas. |
| Após os erros do display | Trinta e seis amostras a partir de 14:51:00 até o encerramento, sem novos erros registrados. O responsável confirmou o retorno normal da tela de leituras após reconectar o OLED. |

Os códigos registrados correspondem a erro de comunicação dos componentes segundo o roteiro. Não há motivo para descrevê-los como timeout.

Os horários de confirmação do assistente são referências de execução no computador e não medem o instante físico exato da intervenção.

## Medição corrigida do LED

O `digital.csv` cobre aproximadamente 0–233,08 s e contém operação normal, transições e erros dos periféricos. A tabela anterior reunia 37 ciclos dessa sequência inteira. Entre eles havia um nível alto de aproximadamente 104 s, correspondente a uma condição diferente da piscada estável.

Por isso, a média anterior de **5.178,416 ms** não representava a periodicidade do LED durante uma falha estável. O registro ativo passou a usar exclusivamente a janela de erro selecionada, mantendo o CSV bruto completo e a tabela anterior como histórico.

Recalculando somente os dez ciclos completos entre as subidas em **151,485141 s e 161,4848195 s** do CSV:

| Medida | Média (ms) | Mínimo (ms) | Máximo (ms) |
| --- | ---: | ---: | ---: |
| Alto | 499,995 | 499,967 | 500,011 |
| Baixo | 499,973 | 499,960 | 499,985 |
| Período | 999,968 | 999,948 | 999,973 |

Essa janela está contida no trecho de erro do sensor e exclui o ciclo de entrada e a recuperação. O cálculo corrigido está em `led_intervalos_20261107_152333_291746.csv`; seus limites, estatísticas e fontes estão em `sessao.json` e `observacoes.md`. A correção foi feita com os dados existentes, sem nova aquisição. A análise dos canais internos do `.sal`, incluindo condição do botão nessa janela, continua sendo conferência do responsável.

O formulário **Medir LED no CSV…** agora permite informar início e fim da janela no CSV inteiro. A conferência automática recalcula apenas os ciclos completos contidos nesses limites.

`TIMER=LIMITADO` continua apropriado: a janela mede a sinalização do LED e não mede diretamente o período das interrupções de 1 ms. Os valores acima não são uma avaliação da exatidão do relógio nem da precisão do instrumento.

## Observações registradas e referências da captura

Foram registrados em `notes` e no relatório da sessão:

- A remoção completa e reinserção dos módulos como método da campanha.
- O relato do botão: a tela mudou para diagnóstico ao pressionar; ao manter pressionado, não houve novas trocas nem outras ações. A duração elétrica não foi medida nesta revisão.
- O relato da recuperação do OLED: a tela de leituras voltou a ser exibida normalmente após a reconexão.
- A janela corrigida do LED, sua referência ao CSV/`.sal` e a exclusão dos demais ciclos do cálculo.

Um intervalo do `.sal` é o início e fim de um trecho, em segundos na linha do tempo do Logic 2. A janela do LED, por exemplo, é `151,485141–161,4848195 s`. Para I2C, faltam as posições efetivamente consultadas para conferir `0x44`/`0x3C`, comando `0x24 0x00`, STOP e leitura separada de seis bytes. Para UART, falta localizar o trecho onde foram conferidos os registros e CRLF. Esses tempos não foram inventados nem deduzidos dos cliques; as aprovações permanecem as informadas pelo responsável.

Não são necessárias fotos extras por padrão. As observações do responsável e a localização dos trechos existentes podem complementar o registro. Se algum comportamento não foi observado, a conclusão correspondente precisa permanecer limitada ou pendente.

Nesta análise foram examinados os metadados e a estrutura do `.sal`, as fotos, os arquivos UART, os CSVs e os registros da sessão. **Os sinais I2C/UART/botão internos do `.sal` não foram decodificados independentemente.** As aprovações desses grupos permanecem declarações do responsável até a conferência dos trechos. Não há marcadores nem notas salvos nos metadados da captura.

A contagem documental final de UART foi recalculada do `.bin` atual: **8.426 bytes**, 134 amostras, seis erros do sensor e 16 erros do display. O texto reconstruído do `.log` atual contém **8.510 bytes** e nove erros do sensor. Portanto, os dois arquivos atuais divergem, e não devem ser descritos como cópias idênticas da recepção. Ambos foram preservados; a causa da diferença não foi inferida. A contagem anterior de 9.192 bytes e nove erros do sensor foi preservada em `stats_at_close`, assim como os contadores históricos das confirmações. Essa divergência não é detectada pela conferência automática atual, que verifica a presença dos arquivos e as amostras do binário.

## Preparação editorial do passo 7

Para esta versão da ESP32, o formato já permite preparar uma linha da tabela funcional, a síntese de conformidade com as limitações, figuras das telas e um trecho de UART com amostra, erro e retomada. A periodicidade já usa a janela corrigida.

Para fechar o passo 7 completo ainda são necessárias a campanha das demais versões, as tabelas consolidadas, a seleção das figuras em `tcc_2/figuras/`, o diagrama da arquitetura e as notas de discussão. São atividades de consolidação previstas no roteiro, não ausência de um novo tipo obrigatório de anexo nesta sessão.

O ajuste prioritário para repetir o formato nas próximas placas é: descrever a remoção/reinserção, confirmar as ações próximo ao momento em que ocorrem, manter uma única janela estável de erro com pelo menos dez ciclos e registrar os intervalos efetivamente inspecionados no analisador.
