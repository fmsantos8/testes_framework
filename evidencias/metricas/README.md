# Métricas computacionais — passo 6

Coleta dos oito builds finais identificados no passo 2. O passo 6 não envolve gravação ou execução de placas. Os fontes e binários foram preservados; todos os ensaios físicos continuam pendentes.

**Comece pelo [RESUMO.md](RESUMO.md)**, que reúne as tabelas e a interpretação dos resultados. As regras, exclusões e dados para conferir cada número estão neste diretório.

## Arquivos principais

| Arquivo | Conteúdo |
| --- | --- |
| [inventario.csv](inventario.csv) | Uma linha por projeto/C ou H realmente usado: categoria, origem mantida, LOC, hash, papel no build, camada e dependência direta de SDK. |
| [codigo.csv](codigo.csv) | Totais por firmware: comum, específico, Pcomp, controle de formatação, conteúdo idêntico e dependências. |
| [codigo-mantido.csv](codigo-mantido.csv) | Arquivos/versões por origem, contados uma única vez no conjunto de quatro placas. |
| [identidade-conteudo.csv](identidade-conteudo.csv) | Arquivos com mesmo SHA em duas ou mais placas, distinguindo as origens mantidas. Inclui as cópias convencionais. |
| [dispersao.csv](dispersao.csv) | Distribuição das dependências diretas por diretório. |
| [dispersao-camadas.csv](dispersao-camadas.csv) | Distribuição por responsabilidade: aplicação, dispositivos/componentes, contratos, arena, porte, BSP e integração. |
| [dependencias-sdk.json](dependencias-sdk.json) | Inclusões e identificadores encontrados, com linha no arquivo, para revisão da classificação. |
| [categorias.csv](categorias.csv) | Contagens e LOC de cada categoria, incluindo o que foi excluído. |
| [reuso-stm32.csv](reuso-stm32.csv) | Identidade, origem e LOC dos sete C/H do porte reutilizado em F407/F411. |
| [memoria.csv](memoria.csv) | Flash e RAM das oito versões, configuração e origem do número. |
| [comparacao-memoria.csv](comparacao-memoria.csv) | Diferenças absolutas e percentuais dos quatro pares, usando a convencional como referência percentual. |
| [regras.json](regras.json) | Regras explícitas de seleção, contagem, origem, exclusões, dependências e memória. |
| [resultados.json](resultados.json) | Consolidação, ferramentas, comandos e hashes dos dados de entrada/saída. |
| [verificacao.json](verificacao.json) | Resultado da conferência de integridade, fórmulas, exclusões, origens e correspondência com os builds. |

Os CSVs são UTF-8, separados por ponto e vírgula. Números não têm separador de milhar; percentuais usam ponto decimal. Configure a importação da planilha dessa forma. A síntese Markdown apresenta os números em português.

## 1. Seleção e contagem de código

O inventário combina os fontes de `compile_commands.json` com os headers efetivamente registrados por `ninja -t deps` nos oito builds da aplicação. As dependências originais estão comprimidas, sem perda, em `dados/<projeto>-dependencias.txt.gz`. São consideradas apenas extensões `.c` e `.h`, conforme o recorte da proposta. Não se presume uso de um arquivo apenas porque ele está presente no diretório.

As linhas são contadas com **cloc 2.06**, por arquivo, com `--skip-uniqueness`. Isso evita que arquivos idênticos sejam eliminados silenciosamente da contagem por placa; o desconto de compartilhamento ocorre em uma etapa explícita. `LOC` corresponde à coluna `code`: comentários e linhas vazias são separados, diretivas contam como código. Os arquivos completos são contados, incluindo funções e condicionais que podem ser eliminadas no binário. Três entradas de tamanho zero omitidas pelo cloc foram registradas com zero linhas.

O bruto do contador está em `dados/cloc.json`, com log e lista dos arquivos em `dados/`. As listas `arquivos-<projeto>.txt` contêm apenas os arquivos elegíveis à métrica autoral de cada versão, com caminhos relativos à raiz do workspace.

### Exclusões de LOC

- SDKs, CMSIS, drivers de fabricante e biblioteca C/toolchain são inventariados e excluídos da métrica autoral.
- Tabelas de fontes em `cmp_ssd1306_fonts.c` e nas cópias `fonts.c` são identificadas como geradas e excluídas. Seus bytes continuam presentes no consumo de memória.
- `syscall(s).c` e `sysmem.c` com declaração de geração automática ST são excluídos nas duas formas de implementação.
- `Core/Src/system_stm32f4xx.c` é cópia de CMSIS: sua identidade com o template do SDK fixado é conferida antes da exclusão.
- `main.c`, inicialização GPIO/I2C/UART/timer, MSP, interrupções e configuração HAL escritos no trabalho são contados. Não foi excluído todo o diretório `Core/` como se fosse gerado.
- Assembly/startup, linker scripts, arquivos CMake, instaladores, testes, ferramentas de análise e produtos de compilação não integram a métrica C/H autoral. Continuam necessários à reprodução e não são tratados como código inexistente.

As categorias e motivos estão disponíveis por arquivo, de modo que as decisões possam ser revisadas sem reconstruir os firmwares.

## 2. Compartilhamento, cópias e código mantido

`Pcomp = 100 × Lcomum / (Lcomum + Lespecifico)` é calculado separadamente para cada placa e forma de implementação. São comuns os arquivos mantidos na mesma origem, com mesmo SHA e utilizados nas quatro placas, sem condicionais específicas de SDK/placa.

Uma origem é identificada pelo repositório Git e caminho do arquivo dentro dele. Os checkouts dos submódulos comuns apontam para a mesma origem. Arquivos convencionais em `esp32_sem_framework/`, `nrf52840_sem_framework/` e nos dois projetos STM32 são caminhos mantidos separadamente, mesmo quando seus conteúdos são iguais.

Nesta árvore, há **44 C/H autorais comuns e 1.815 LOC** nas versões com framework. Considerando também a tabela de fontes gerada, os 45 C/H comuns finais são idênticos. Isso demonstra identidade entre as árvores finais, sem provar que esses arquivos nunca foram editados durante o desenvolvimento.

O Pcomp estrito convencional é zero pela organização atual das origens. Entretanto, **11 arquivos, com 486 LOC por projeto, são idênticos nas quatro versões convencionais**. As colunas de identidade por conteúdo e o CSV específico tornam esse resultado visível. Não se deve usar o zero para afirmar que uma implementação convencional não permite reúso. A migração das cópias para uma origem compartilhada mudaria essa métrica sem necessariamente mudar seu comportamento.

Código mantido é contado uma vez por par `(origem, SHA)`, sem multiplicar um submódulo idêntico pelos quatro checkouts. Uma origem com versões diferentes mantidas simultaneamente conta cada conteúdo. Cópias independentes permanecem separadas. O porte STM32 é específico na métrica global e compartilhado entre as duas placas na evidência intrafamília: sete arquivos, 1.080 LOC.

### Controle da influência da formatação

As versões convencionais usam frequentemente várias instruções na mesma linha. Foi acrescentada uma medida complementar: cópias dos C/H autorais foram formatadas com o mesmo clang-format e estilo, e contadas pelo mesmo cloc. Os fontes compilados não foram formatados nem alterados.

Ferramenta: Espressif clang-format 21.1.3, revisão `esp-21.1.3_20260408`. Configuração em `dados/estilo-clang-format.json`, cópias em `dados/normalizado/` e contagem em `dados/cloc-normalizado.json`. Marcas `clang-format off/on` foram removidas apenas das cópias. Os dois conjuntos de números são preservados em `codigo.csv` e `codigo-mantido.csv`; os resultados normalizados não substituem os originais.

LOC, quantidade de arquivos e percentuais de reúso não medem diretamente complexidade, produtividade ou esforço de manutenção. O framework tem mais linhas autorais mantidas neste experimento, inclusive no controle de formatação, embora reduza a repetição por origem.

## 3. Dependências dos SDKs

A classificação é por arquivo autoral: inclusão direta de header de SDK/RTOS/CMSIS ou uso de identificadores dessas APIs, tipos e configurações. Comentários, strings de apresentação, caminhos em `#include` ao buscar identificadores e nomes de macros definidos localmente — incluindo guardas de header — são desconsiderados como falsos positivos. Biblioteca C padrão não é classificada como dependência de fabricante.

As correspondências e linhas foram disponibilizadas em `dependencias-sdk.json` e revisadas nos arquivos marcados. É uma regra de análise estática por inclusões/identificadores, não uma prova formal de independência nem análise de toda dependência transitiva. Diretivas/funções presentes no fonte completo podem ser contadas mesmo que inativas na configuração.

Nos quatro projetos com framework, as dependências diretas encontradas ficam em portes, BSP e integração. Aplicação, componentes, interfaces e arena não apresentam correspondências. Nas convencionais, aparecem também na aplicação e nos módulos de dispositivos e telemetria.

As tabelas preservam números absolutos e percentuais. O framework tem mais arquivos com SDK em termos absolutos no ESP32/nRF e menos nos STM32. A quantidade de diretórios depende da organização; a interpretação principal é a localização das dependências e a preservação das fronteiras, não um único contador isolado.

## 4. Memória

São usados os BIN, ELF e MAP **arquivados no passo 2**, identificados por hash, sem compilar outra revisão ou executar placas. A configuração de cada par já foi conferida no passo 2: `-Og`, assertions habilitadas e opções equivalentes de SDK, biblioteca C, serviços e reservas.

### Flash

`flash_bytes` é o tamanho da imagem BIN **da aplicação**, que representa os bytes dessa imagem a gravar. Não é o tamanho de ELF, HEX, UF2 ou da tabela de partições. Inclui código, constantes, inicialização de RAM, alinhamento e, no ESP32, empacotamento de segmentos/cabeçalho/hash. Exclui o bootloader separado, tabela de partições, NVS e áreas reservadas fora da imagem. No nRF, a aplicação começa em `0x26000`; a região anterior e o bootloader não são somados.

Como conferência independente, nos STM32/nRF o tamanho do BIN coincide com a ocupação FLASH obtida dos endereços físicos dos segmentos ELF e da origem do MAP. No ESP32, o payload inicializado dos segmentos ELF coincide com `Total image size` do relatório ESP-IDF. A diferença entre payload e BIN é publicada nos dados detalhados como alinhamento/empacotamento.

### RAM

Para cada região RAM do MAP, mede-se o fim da última seção alocada menos a origem da região. Somam-se apenas regiões com ocupação. São incluídos dados, BSS, `noinit`, alinhamento e reservas de heap/stack já representadas nas seções; não se somam novamente os valores de configuração.

Nos STM32/nRF, a soma coincide com o uso de RAM informado pelo linker. CCMRAM não utilizado conta zero. No ESP32, IRAM com código residente também é RAM; áreas de flash mapeadas em IROM/DROM são excluídas desse total. RTC slow normal e reservada são regiões separadas, evitando contar o espaço livre entre elas. O ELF inclui três bytes adicionais omitidos da síntese ESP-IDF: dois de `.noinit` e um de alinhamento em `.iram0.text_end`.

**Não é medição de RAM em execução:** alocações dinâmicas dos drivers/RTOS, incluindo stacks criadas dinamicamente no ESP32, não são inferidas desses números. Reservas presentes no ELF, como as stacks estáticas do Zephyr ou a reserva declarada no linker STM32, entram no total. Os resultados devem ser comparados principalmente dentro de cada par, sem concluir pico de RAM ou custo dinâmico total.

Os arquivos `dados/<projeto>-memoria.json` preservam regiões, seções, segmentos, payload, fontes e hashes. O framework ocupa mais flash e RAM estática/reservada nos quatro pares. Isso cobre a implementação completa — validações, capacidades, contextos e representação dos recursos —, não apenas o custo de uma chamada indireta.

## 5. Como repetir e conferir

Com as dependências do passo 2 instaladas, execute na raiz:

```bash
build/deps/venv/bin/python tools/coletar_metricas.py > evidencias/metricas/execucao.log 2>&1
python3 tools/verificar_metricas.py
```

O coletor usa pyelftools do ambiente local, Ninja das ferramentas registradas em `evidencias/firmwares/ambiente.json`, o formatter indicado acima e cloc em `build/deps/cloc-2.06.pl`. O cloc foi obtido de `https://raw.githubusercontent.com/AlDanial/cloc/v2.06/cloc`; seu SHA está em `resultados.json`. Nenhuma dependência é baixada automaticamente durante a coleta.

Em outro ambiente, restaure as versões e ajuste os caminhos das ferramentas de modo explícito. O coletor verifica os hashes dos fontes e dos artefatos antes de medir. Uma nova coleta substitui os resultados e as cópias de formatação; preserve esta pasta se quiser manter uma rodada anterior. A verificação falha se os arquivos, fórmulas, exclusões ou snapshots deixarem de corresponder aos dados registrados.

O artigo e o TCC1 não foram alterados nesta etapa. A redação permanece para depois da coleta física. As tabelas desta pasta já podem ser usadas na preparação posterior do artigo.

### Conferência após o início da bancada

O verificador aceita o avanço da campanha em `controle.csv`, mantendo a conferência da identidade, configuração, ELF e hash das oito versões e exigindo data válida nos resultados já registrados. Uma aprovação física não invalida as métricas; os ensaios físicos não são executados por esse comando.

Na revisão de 08/11/2026, os arquivos `cmake/flags.cmake` do STM32F407 e STM32F411 com framework foram alinhados aos hashes exatos do manifesto dos builds arquivados: `-Og -g3` e newlib-nano. Nenhum binário foi recompilado ou substituído nessa revisão. Os fontes e os arquivos de build conferidos voltaram a corresponder aos snapshots; a verificação de fórmulas, saídas, entradas e artefatos permanece integral.
