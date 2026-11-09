# Roteiro comum de conformidade dos portes

Esta entrega atende à disponibilização do roteiro e dos resultados de P-02 do TCC2. Avalia os contratos atuais de sistema, GPIO, I2C, UART e temporizador nas quatro integrações com framework. O texto do TCC1 permanece como referência histórica; não é objeto de atualização.

## Artefatos

- [Capacidades e restrições](CAPACIDADES.md): versão dos contratos, limites de cada porte e seleção dos BSPs.
- [Runner](run.py): compila os portes e BSPs reais e executa o conjunto comum de casos no Linux.
- [Casos comuns em C](host/common.c): as mesmas expectativas públicas para os quatro alvos.
- [Matriz desta execução](resultados/MATRIZ.md): resultado por caso e plataforma.
- [Resultados completos](resultados/resultados.json): revisões, hashes, comandos/logs e limites da evidência.
- [Modelo para registros de bancada](bancada-modelo.json): preenchimento posterior por placa.

## Executar sem placa

Requer Linux, Python 3.10 ou posterior, compiladores C/C++ com C11/C++11 e UBSan, Git e uma instalação externa do STM32CubeF4. Não instala dependências nem acessa a rede.

Na raiz do workspace:

```sh
python3 conformidade/run.py --stm32-sdk /caminho/STM32CubeF4
```

Também aceita `STM32F4_SDK_DIR`. Na ausência desses dois parâmetros, tenta a instalação registrada no manifesto de compilação selecionado. Por padrão, utiliza `evidencias/firmwares/manifesto.json` quando disponível; caso contrário, usa o histórico em `build/compilacao/manifesto.json`. A opção `--build-manifest /caminho/manifesto.json` permite selecionar outra rodada. Os dois formatos de manifesto são aceitos. Os testes ESP32/nRF52840 usam modelos locais dos serviços do SDK e não exigem esses SDKs no host. A vinculação com os SDKs reais é evidenciada separadamente pelos builds de produção.

Para preservar rodadas anteriores, escolha outro diretório de resultados:

```sh
python3 conformidade/run.py --output conformidade/resultados/nova-rodada
```

O runner grava os comandos e saídas em `logs/`, gera `resultados.json` e `MATRIZ.md`, e retorna código diferente de zero se houver falha ou bloqueio nos testes automatizados, ou divergência nos builds de produção conferidos. Para esses builds, compara os fontes ensaiados, o snapshot de fontes, os hashes dos artefatos e a revisão do SDK STM32 quando registrada. `NAO_APLICAVEL` não é aprovação. Casos físicos permanecem `NAO_EXECUTADO` no relatório automático e não fazem o runner passar a declarar conformidade física.

Cada caso roda em um processo novo: a arena e as reservas dos portes são permanentes e não possuem uma API de reset para testes. Não são fabricados handles não nulos inválidos para provocar acesso a memória arbitrária.

## Alcance dos testes automatizados

| Grupo | Verificação |
| --- | --- |
| SYS-01 a SYS-04 | Uso antes de inicializar; argumentos/frequência; inicialização única; construção/selagem; arena insuficiente |
| CFG-01 a CFG-03 | Ponteiros, enums, pinos, timeout zero, período zero e configurações indisponíveis; ausência de consumo nos casos rejeitados antes da alocação |
| RES-01 | Rejeição de recursos repetidos e preservação do consumo |
| GPIO-01 | Níveis físicos, direção e argumentos de leitura/escrita |
| I2C-01 a I2C-05 | Argumentos e limites; leitura/escrita separadas; tradução de erro/timeout; nova chamada; falha de construção sem publicar handle |
| UART-01 a UART-06 | Transmissão vazia/completa; contagem; erro; timeout sem progresso; prefixo parcial; bloco de 129 bytes |
| TIMER-01 a TIMER-03 | Argumentos; callback antes/depois de iniciar; início após selagem; início repetido; configuração indisponível quando existente |
| BSP-01 | Composição do BSP real, polaridades, recursos, selagem e temporizador inicialmente parado |
| Estrutura/compilação | Identidade dos arquivos comuns/porte STM32; cabeçalhos C11/C++11 sem SDK; hashes e fontes dos builds de produção disponíveis |

Os casos são definidos em `CASES` de `run.py`. Cada ID corresponde ao ramo de mesmo nome em `host/common.c`, com asserções explícitas e falha imediata. Não há testes apenas de uma HAL falsa: os símbolos públicos `hal_*` são os próprios fontes dos portes e a arena real.

### O que é simulado

**ESP32:** os cabeçalhos e serviços utilizados do ESP-IDF/FreeRTOS são substituídos pelo modelo em `host/esp32`. O porte, seus validadores, tradução de estados, controle de recursos e ciclo de vida são reais. O modelo fornece sucesso, erro, timeout, aceitação parcial na FIFO e eventos do temporizador. Não comprova ABI do SDK, funcionamento do driver ou agendamento do FreeRTOS.

**nRF52840:** o modelo em `host/nrf52840` substitui Zephyr/nrfx, semáforos, pinctrl, clock e eventos EasyDMA. São reais o porte, seus buffers persistentes, tratamento do timeout, contagem e lógica de abort. O modelo é sequencial; não comprova corridas de interrupção, evento tardio, latência do abort nem transições físicas de DMA. Esses caminhos exigem ensaio específico ou devem permanecer como limitação declarada.

**STM32F407 e STM32F411:** cada construção usa seus fontes reais e sua macro de dispositivo. O shim de CPU substitui CMSIS Core/NVIC; os registradores ST são mapeados em memória Linux e os drivers reais de GPIO, configuração I2C/UART e temporizador são compilados. Clocks e passagem de tempo são simulados. As transferências I2C/UART são interceptadas na fronteira do SDK para injetar resultados e contagens. Não há simulação elétrica nem emulação completa dos efeitos dos registradores. O shim de CPU foi reaproveitado e adaptado do teste histórico do F411, anterior ao commit `265c478`; os casos atuais foram escritos contra os contratos vigentes.

As verificações de timer no host avaliam configuração e entrega lógica de eventos. O tempo do host, os ticks do modelo e os timeouts injetados não são medições do tempo físico. As variantes e limites não exercitados continuam descritos em `CAPACIDADES.md`.

## Roteiro de bancada comum às quatro placas

Identificar a revisão física da placa, firmware e sua configuração, hashes dos binários, versões dos SDKs, montagem, instrumentos, critérios adotados, responsável e data. Usar os mesmos dispositivos, pinos e polaridades da [declaração de capacidades](CAPACIDADES.md).

O firmware de aplicação disponível serve para observar comportamento externo; a leitura de retornos HAL e a marcação temporal podem exigir uma compilação de diagnóstico da integração. Esta entrega não gera nem grava um firmware instrumentalizado de bancada. Instrumentação adicional deve ser identificada e excluída dos binários usados para comparar memória.

Os ensaios de falha I2C abaixo começam somente após inicialização bem-sucedida. Não é necessário suportar sensor/display ausente desde a partida.

### HW-01 — GPIO e estado inicial

1. Capturar o pino do LED durante reset, construção dos recursos e início da aplicação; observar também o botão solto e pressionado.
2. Confirmar nível alto com botão solto e baixo quando pressionado, de acordo com o circuito de pull-up externo.
3. Confirmar LED inativo durante a inicialização e ativo durante operação normal. A HAL não promete ausência absoluta de glitches; registrar qualquer transiente observado.
4. Registrar montagem, captura e correspondência entre leitura/escrita lógica e nível físico.

**Aceitação:** correspondência dos níveis e polaridades com o BSP; nenhum comportamento externo contrário ao estado definido para a prova de conceito. Limitações de resolução da captura devem constar do registro.

### HW-02 — I2C normal e STOP

1. Após inicialização válida, capturar SDA/SCL com decodificação I2C.
2. Identificar o comando SHT31-D `0x24 0x00` em `0x44`, encerrado com STOP, e a leitura separada de seis bytes após a conversão.
3. Identificar escritas em `0x3C` para o SSD1306 no mesmo barramento, sem acesso concorrente.
4. Registrar a frequência medida, bytes, ACK/NACK, STOPs e intervalo entre comando/leitura. Conferir os dois CRCs e os valores publicados.

**Aceitação:** transferências normais com endereços, tamanhos e sequência esperados; espera compatível com a conversão; frequência e níveis registrados dentro dos limites dos dispositivos utilizados. Ausência de transação combinada não é falha.

### HW-03 — Erro I2C, timeout e retomada

1. Obter ao menos uma aquisição válida.
2. Provocar NACK do sensor sem indisponibilizar o display, por exemplo interrompendo apenas o ramo de comunicação do sensor. Registrar o retorno HAL por instrumentação diagnóstica e o estado visível na aplicação.
3. Restaurar a ligação e observar uma nova aquisição válida.
4. Em ensaio separado, usar um mecanismo controlado de retenção em nível baixo de SDA ou SCL para exercitar espera do driver. Registrar retorno, duração medida e o estado da chamada seguinte após liberar o barramento.
5. Preservar os registros UART e as capturas de LED/display, relacionando-os aos retornos observados.

**Aceitação:** os códigos são compatíveis com o evento e com o contrato/SDK; não ocorre sucesso falso; a última amostra válida é preservada e o erro é observável. O timeout configurado não deve ser tratado como prazo estrito de toda a chamada. Se determinado defeito não puder ser induzido, marcar o subensaio como não executado e registrar a ameaça à validade.

### HW-04 — UART normal

1. Capturar TX por adaptador e, quando disponível, analisador lógico a 115200 8N1.
2. Relacionar o bloco solicitado, o retorno e a contagem aceita com os bytes transmitidos. Para a prova de conceito, conferir registros `SAMPLE` e `ERROR` terminados por CRLF.
3. Confirmar o término do fluxo normal antes de declarar a transmissão concluída; recepção pela outra ponta não faz parte da garantia da HAL.

**Aceitação:** formato, ordem e bytes corretos, contagem igual ao tamanho no sucesso e ausência de referência ao buffer do chamador após o retorno.

### HW-05 — Temporizador e contador de notificações

1. Em build diagnóstico, marcar a ativação do timer e as entradas no callback em pino(s) livre(s), com pulsos curtos e sem chamadas bloqueantes no callback.
2. Confirmar ausência de callbacks antes do início e observar o primeiro evento, levando em conta a latência até o atendimento da interrupção.
3. Capturar o período configurado de 1 ms e comparar um intervalo de referência externa com o incremento do contador.
4. Repetir com tráfego de sensor/display e falhas I2C, registrando atraso e eventual aglutinação. Repetir `start` e conferir `INVALID_STATE`, sem reinício da fase.

**Aceitação:** período configurado e ciclo de vida coerentes; desvios medidos e condições de carga registrados. Uma contagem menor de notificações atendidas não deve ser ocultada nem convertida em promessa de tempo físico exato. Não há requisito de recuperar períodos perdidos.

### HW-06 — UART parcial e timeout

1. Usar instrumentação de driver/depuração ou mecanismo de falha justificável para impedir a conclusão da transmissão após zero bytes e após um prefixo conhecido.
2. Registrar retorno, contagem aceita, bytes efetivamente observados e possível saída de bytes após a falha. Conferir que o buffer do chamador pode ser reutilizado.
3. No nRF, observar também a conclusão do abort e a política de indisponibilizar o recurso quando ele não termina no limite adicional. Não inferir validação desse caminho apenas do sucesso do abort simulado.

**Aceitação:** nenhum sucesso falso, contagem compatível com o prefixo aceito e sem uso posterior do buffer do chamador. Caso a falha não possa ser reproduzida no hardware, manter essa limitação explícita. Desconectar o receptor UART não provoca necessariamente timeout: a interface não tem confirmação remota nem controle de fluxo no contrato mínimo.

## Registro e conclusão

Copiar `bancada-modelo.json` para um arquivo por placa e anexar as evidências. Não editar os resultados automatizados para transformar testes simulados em físicos. O campo `full_physical_conformance_demonstrated` permanece falso nesta rodada.

O roteiro e a matriz estão disponibilizados. A aprovação simulada permite avaliar os caminhos exercitados dos portes; a conformidade física plena permanece dependente dos ensaios de bancada ou de limitações assumidas explicitamente no artigo final. Não há placas seriais disponíveis nesta sessão para executar esses ensaios.
