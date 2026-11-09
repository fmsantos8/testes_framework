# Material do passo 7 para o artigo

Tabelas, figuras e notas preparadas em 09/11/2026 a partir dos oito firmwares identificados e da campanha de 07–08/11. O artigo será redigido no passo 8, usando o template existente em `tcc_2/`.

## Tabelas e registros selecionados

- [TABELAS.md](TABELAS.md): funcionamento das oito versões, conformidade física e simulada, código/dependências, controle de formatação, código mantido por origem, flash, RAM, LED e conferência dos sinais. Cada tabela aponta para os dados de origem e para um `tabular` em [tcc_2/tabelas](../../tcc_2/tabelas/).
- [UART.md](UART.md): recortes literais do STM32F411 com framework, incluindo leitura, falha do sensor, recuperação, falha do display com aquisição mantida e recuperação. As linhas de origem e as lacunas editoriais estão explícitas; [uart.csv](uart.csv) permite conferir cada linha.
- [LIMITACOES.csv](LIMITACOES.csv): limites que devem acompanhar a apresentação dos resultados, com referência às sessões.
- [OBSERVACOES.md](OBSERVACOES.md): lista de observações para a redação, relacionando conclusões, números e evidências.
- [conformidade.csv](conformidade.csv), [led.csv](led.csv) e [sinais.csv](sinais.csv): consolidações numéricas sem arredondamento de apresentação, com casos, janelas e arquivos identificados.

Nas tabelas funcionais, a aprovação é a publicada pelo responsável nas sessões. TIMER permanece `LIMITADO` nas quatro placas. Os casos simulados são apresentados separadamente dos físicos. As versões convencionais têm conteúdo idêntico que permanece contado como origens independentes na métrica estrita de reúso. O aumento de memória e de código acompanha a apresentação do compartilhamento e das fronteiras de SDK.

## Figuras finais

| Arquivo | Seleção e legenda proposta | Uso no template |
| --- | --- | --- |
| [montagens.pdf](../../tcc_2/figuras/montagens.pdf) | Montagens da prova de conceito: (a) ESP32, (b) nRF52840, (c) STM32F407 e (d) STM32F411, com SHT31-D, SSD1306, botão e LED. Fotos das sessões com framework; a seleção ilustra a montagem, sem substituir a validação funcional. | Largura de duas colunas, `\textwidth`. |
| [telas.pdf](../../tcc_2/figuras/telas.pdf) | Telas comuns da aplicação na ESP32 com framework: (a) temperatura e umidade; (b) diagnóstico da última aquisição e tempo ativo nominal. Fotografias de instantes distintos da sessão, com valores ambientais ilustrativos. | Largura de uma coluna, `\columnwidth`. |
| [arquitetura.pdf](../../tcc_2/figuras/arquitetura.pdf) | Arquitetura implementada: aplicação e componentes comuns usam contratos da HAL; portes integram SDKs e arena C11; BSP e `main` compõem cada placa. Setas contínuas indicam uso, exceto a seta ascendente identificada como implementação; tracejadas indicam composição. | Largura de duas colunas, `\textwidth`. |

[selecao-figuras.json](selecao-figuras.json) identifica foto, painel, rotação e janela de recorte em pixels, com origem no canto superior esquerdo. Os originais permanecem em `evidencias/bancada/`; os PDFs incorporam as imagens originais com recorte de enquadramento e rotação rígida, sem reconstrução de pixels ou alteração de valores exibidos. O painel do STM32F411 exclui a placa STM32F407 vizinha para identificar a montagem correspondente. O nome original `00_montagem_substituir.jpeg` do nRF foi preservado: a fotografia selecionada contém a montagem legível, rotacionada no PDF.

O diagrama tem fonte editável em [arquitetura.mmd](../../tcc_2/figuras/arquitetura.mmd) e desenho vetorial no gerador. A composição foi conferida contra os quatro `src/main.c`, BSPs, contratos e módulos atuais. O buffer persistente da arena está nos BSPs atuais; `main` cria contextos de componentes, injeta recursos/função de tempo e inicia o temporizador. A integração usa contagem nominal de callbacks; a figura não apresenta uma API de consulta de tempo físico nem uma abstração de RTOS.

Uma imagem adicional do analisador não foi selecionada: a tabela de sinais, os intervalos das quatro sessões e a seleção textual da UART explicam os resultados sem uma figura redundante. Os `.sal` completos e os relatórios de decodificação permanecem disponíveis para localizar comando, STOP, leitura de seis bytes e CR LF. Não é uma captura nova nem uma tela reconstruída do Logic 2.

## Reprodução e conferência

O gerador usa Python 3.12 ou posterior, ReportLab e Pillow. Os dados das tabelas vêm dos CSVs e JSONs existentes, sem nova compilação, gravação ou ensaio físico:

```bash
python3 tools/preparar_material_artigo.py
python3 tools/preparar_material_artigo.py --check
```

A opção `--check` usa somente a biblioteca padrão. Confere os hashes disponíveis de entrada do passo 6, os totais por arquivo autoral, fórmulas, memória contra BIN/ELF, associação entre sessão e firmware, resultados funcionais/físicos, estatísticas dos intervalos do LED, relatórios das capturas, recortes literais da UART e integridade das saídas. Instalações externas antigas de SDK/toolchain, parte do SDK em `build/deps/` e alguns headers/fontes gerados dos builds já não estão presentes: esses arquivos, excluídos de LOC autoral, continuam identificados pelo inventário e pelas contagens preservadas do passo 6, sem uma nova conferência de seus bytes. Todos os fontes autorais e os dados usados nas tabelas permanecem verificáveis. As quantidades conferidas/ausentes estão em `upstream_input_check` do [manifesto.json](manifesto.json), junto dos hashes, gerador e revisão de partida. A revisão de partida antecede estes arquivos; o manifesto identifica o material preparado ainda sem commit próprio.

Um header gerado do build atual do nRF (`devicetree_generated.h`) também difere do hash histórico. Ele é excluído de LOC autoral; o fechamento usa a configuração arquivada e os binários identificados do passo 2, sem atribuir os resultados a uma nova compilação da árvore atual. O caminho está registrado em `different_excluded_non_authoral` no manifesto.

Os três PDFs foram renderizados com Poppler e inspecionados visualmente nos tamanhos previstos. Os `tabular` preservam o ambiente IEEEtran e não alteram o template. A compilação LaTeX e a paginação do artigo pertencem ao passo 8; este ambiente não dispõe de `pdflatex`/`latexmk`.

Condição de conclusão: as seis atividades do passo 7 estão atendidas. Os originais, firmwares e resultados publicados foram preservados.
