# Resultados do passo 3 — testes em computador

Execução concluída em 02/11/2026. As duas suítes terminaram com código de saída zero. Não houve gravação de placas nem execução de ensaios físicos.

| Verificação | Resultado |
| --- | --- |
| Equivalência funcional | 12 comparações aprovadas; nenhuma divergência nas traces. |
| Conformidade dos portes | 95 verificações aprovadas em simulação; uma não aplicável; nenhuma falha ou bloqueio. |
| Cabeçalhos públicos sem SDK | Oito compilações aprovadas: C11 e C++11 em cada uma das quatro integrações. |
| Fontes comuns | Aplicação, componentes, interfaces e arena idênticos nas quatro integrações. |
| Porte STM32 | Fontes idênticos nas integrações F407 e F411. |
| Correspondência com os builds finais | Fontes e hashes relacionados às oito versões preparadas no passo 2; os quatro builds com framework também foram conferidos pela suíte de conformidade. |

Os programas de teste usam `-Wall -Wextra -Werror` e UndefinedBehaviorSanitizer, com interrupção após erro. Compilador host: GCC 15.2.0 (`cc`, Ubuntu 15.2.0-16ubuntu1).

## Equivalência funcional

Cada implementação convencional foi comparada ao núcleo comum real do framework sob as mesmas entradas simuladas. Antes da comparação, a suíte verificou a identidade dos fontes C/H de aplicação e componentes nas quatro referências.

| Versão convencional | Contador normal | Retorno do contador de 32 bits | Saturação da fila UART |
| --- | --- | --- | --- |
| ESP32 | Aprovado | Aprovado | Aprovado |
| nRF52840 | Aprovado | Aprovado | Aprovado |
| STM32F407 | Aprovado | Aprovado | Aprovado |
| STM32F411 | Aprovado | Aprovado | Aprovado |

Foram comparados bytes e sequência das operações I2C/UART, códigos de erro, transições do LED e instantes nominais fornecidos pelo modelo. Os cenários incluem leituras, CRC inválido, falhas de comunicação, preservação da amostra, debounce, telas, páginas pendentes, repetição serial e descarte/ressincronização após envio parcial.

Arquivos: [relatório JSON](equivalencia/results.json), [log da execução](equivalencia/execucao.log) e traces em `equivalencia/*.trace`. Não houve arquivo de diferença nesta execução.

## Conformidade dos portes

| Integração | Aprovado em simulação | Não aplicável | Falhas/bloqueios |
| --- | ---: | ---: | ---: |
| ESP32 | 23 | 1 | 0 |
| nRF52840 | 24 | 0 | 0 |
| STM32F407 | 24 | 0 | 0 |
| STM32F411 | 24 | 0 | 0 |

O caso não aplicável é `TIMER-03` no ESP32: na configuração modelada não há um período positivo de `uint32_t` declarado indisponível que permita exigir `NOT_SUPPORTED`. Um identificador ou período inválido não substitui esse caso. Ele não foi contado como aprovação.

Foram exercitados os portes, BSPs e arena reais na fronteira controlada dos SDKs. Há verificações de inicialização, validade dos argumentos, construção/selagem, recursos, GPIO, I2C, UART completa/parcial, erros, timeouts e ciclo de vida do temporizador.

Arquivos: [matriz detalhada](conformidade/MATRIZ.md), [relatório JSON](conformidade/resultados.json), [log da execução](conformidade/execucao.log) e comandos/saídas por caso em `conformidade/logs/`.

## Rastreabilidade dos resultados

O runner de conformidade consultava o manifesto histórico em `build/compilacao/manifesto.json`. Foi atualizado para preferir [o manifesto final do passo 2](../firmwares/manifesto.json), aceitar explicitamente `--build-manifest` e os dois formatos de manifesto. Também passou a retornar falha quando os builds conferidos apresentarem divergência. Os casos C e suas expectativas de comportamento foram mantidos.

Os quatro builds com framework tiveram confirmados os fontes ensaiados, o snapshot de fontes e os hashes dos artefatos; a revisão do STM32CubeF4 utilizado na simulação também coincide com a dos builds STM32. Os fingerprints da equivalência foram relacionados aos snapshots das quatro versões convencionais e do núcleo comum nas quatro integrações.

O [resumo JSON](resumo.json) registra essa associação para os oito ELF finais, seus hashes, os resultados agregados e os hashes dos relatórios. Os fontes das aplicações, componentes e portes não foram alterados nesta etapa; os binários do passo 2 continuam válidos como referência para a próxima fase.

## Como repetir

Na raiz deste workspace:

```bash
python3 validacao_sem_framework/run.py --output evidencias/simulacao/equivalencia
python3 conformidade/run.py --build-manifest evidencias/firmwares/manifesto.json --output evidencias/simulacao/conformidade
```

A segunda chamada usa o caminho do SDK STM32 registrado no manifesto final. Se a instalação mudar, informe `--stm32-sdk /caminho/real/STM32CubeF4` ou configure `STM32F4_SDK_DIR`. O compilador C/C++, UBSan e o SDK precisam estar disponíveis. Esses comandos substituem os relatórios nos destinos indicados; escolha outra pasta para preservar esta rodada. O `resumo.json` é a consolidação desta rodada e precisa ser regenerado/conferido se os resultados forem substituídos.

## Limites da evidência

Os executáveis host recompilam os fontes para o computador; não executam os ELF embarcados. A associação aos ELF demonstra identidade dos fontes e dos artefatos registrados, não execução desses binários no microcontrolador.

Na equivalência, os acessos à HAL/SDK são simulados, e `main.c`/inicialização da plataforma não são executados. No nRF convencional, os helpers de `peripherals.c` são substituídos pelo adaptador host. Na conformidade, há modelos de SDK, registradores, clocks e eventos; não são comprovados comportamento elétrico, precisão temporal, corridas de interrupção, efeitos físicos de DMA ou latência real de abort.

Na execução computacional de 02/11/2026, os ensaios físicos ainda estavam pendentes. Por isso, este relatório automático mantém os seis grupos físicos por placa como `NAO_EXECUTADO`. Esse estado histórico não descreve o avanço posterior da bancada. A campanha física atual é registrada em [controle.csv](../controle.csv), [resultados-funcionais.csv](../resultados-funcionais.csv) e [resultados-conformidade.csv](../resultados-conformidade.csv): em 08/11/2026, as oito versões estão aprovadas funcionalmente, com as limitações e os complementos documentais de conformidade indicados nas sessões. Os resultados simulados permanecem separados dos físicos.
