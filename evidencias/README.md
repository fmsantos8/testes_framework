# Evidências do TCC2

Preparação do passo 1 concluída em 02/11/2026. Este diretório reúne os registros que fundamentarão os resultados do artigo. A preparação das pastas e planilhas não representa aprovação dos ensaios.

## Situação da campanha — revisão de 09/11/2026

As oito versões foram ensaiadas: ESP32, nRF52840, STM32F407 e STM32F411, com e sem framework. Os oito campos funcionais de cada uma estão publicados como `APROVADO` (64 campos). A execução da bancada funcional e as referências de I2C/UART estão concluídas. A organização editorial do passo 7 foi concluída em 09/11/2026: [material para o artigo](artigo/README.md), com tabelas conferidas, figuras, seleção de UART, notas e limitações. O passo 8 também foi concluído nessa data: [artigo integral](../tcc_2/README.md), com dez páginas incluindo referências, contratos públicos e trechos reais do BSP, porte e integração, pronto para revisão.

| Placa | Com framework | Sem framework |
| --- | --- | --- |
| ESP32 | APROVADO — 07/11/2026 | APROVADO — 07/11/2026 |
| nRF52840 | APROVADO — 08/11/2026 | APROVADO — 08/11/2026 |
| STM32F407 | APROVADO — 08/11/2026 | APROVADO — 08/11/2026 |
| STM32F411 | APROVADO — 08/11/2026 | APROVADO — 08/11/2026 |

Nas quatro versões ensaiadas com framework, GPIO, I2C e UART estão publicados como aprovação física informada pelo responsável; TIMER permanece `LIMITADO`, pois a periodicidade do LED é evidência indireta. A conferência independente dos sinais I2C/UART das sete capturas `.sal` disponíveis complementa esses registros. Cada sessão com captura contém `conferencia-sal.json`, `intervalos-sal.csv` e as janelas nas observações, em segundos desde o início da gravação. Nas quatro versões com framework, todas as 290 leituras completas têm CRC válido, comando e leitura separados por STOP e valores correspondentes às mensagens UART com CR LF. Nenhum erro de stop bit foi detectado na decodificação a 115200 8N1; isso não mede o baud físico. Os tempos extraídos do LED coincidem exatamente com os CSVs exportados. Reprodução: `python3 tools/conferir_sal.py`, a partir da raiz; os dados detalhados são gerados em `/tmp/sal-decoded`.

Na ESP32 com framework, a captura contém três `ERROR` do sensor entre 112,753343 s e 116,740816 s que não aparecem nos logs UART atuais. Esse bloco adicional permanece preservado na captura e foi excluído da janela principal, que contém os seis erros entre 150,985310 s e 160,973646 s. Nas capturas convencionais, as diferenças de início/fim em relação ao monitor UART foram identificadas; a transferência parcial do OLED na inicialização/reset da ESP32 convencional também foi registrada. As capturas brutas e os logs não foram alterados.

As oito sessões têm firmware arquivado íntegro, arquivos UART atuais de texto/binário coincidentes, fotos e resultados vinculados aos CSVs. As janelas ativas do LED foram recalculadas; os intervalos de saída da falha foram excluídos nas sessões corrigidas. A captura e a medição do LED são exigidas nas versões com framework; sua ausência no STM32F411 convencional não representa pendência. Na versão convencional do STM32F407, os erros simultâneos durante a manipulação adicional das conexões estão registrados nas observações, conforme esclarecimento do responsável. No STM32F411 com framework, o período estável do LED é 1.030,123 ms, cerca de 3,012% acima do nominal; essa diferença está documentada sem atribuí-la ao timer isoladamente.

As datas e os resultados atuais são os dos CSVs abaixo. Os relatórios datados das etapas computacionais descrevem aquelas execuções anteriores; não substituem a bancada. O fechamento automático confere presença e coerência documental, enquanto a interpretação dos comportamentos físicos depende das evidências e das observações do responsável.

## Arquivos de controle

- [controle.csv](controle.csv): oito versões cadastradas, com projeto, configuração, ELF arquivado, hash, data e aprovação funcional física.
- [resultados-funcionais.csv](resultados-funcionais.csv): campos do roteiro funcional por placa/versão, com oito versões aprovadas e nenhuma pendente.
- [resultados-conformidade.csv](resultados-conformidade.csv): grupos físicos das quatro placas ensaiadas com framework, com TIMER limitado e referência separada à simulação.
- [simulacao/README.md](simulacao/README.md): resultados do passo 3, com equivalência, conformidade e associação aos builds finais. A aprovação em simulação não altera os resultados físicos dos CSVs.
- [metricas/RESUMO.md](metricas/RESUMO.md): tabelas do passo 6, com código, reúso, dependências e flash/RAM. Regras, dados por arquivo e reprodução em [metricas/README.md](metricas/README.md).
- [artigo/README.md](artigo/README.md): fechamento do passo 7, com tabelas para LaTeX, três figuras finais, seleção literal de UART e observações para a redação. Os limites físicos/simulados e as diferenças desfavoráveis permanecem associados às evidências.

Os CSVs usam UTF-8 e ponto e vírgula como separador. Na importação em uma planilha, selecione essas opções e mantenha hashes como texto.

## Onde guardar cada material

| Pasta | Conteúdo |
| --- | --- |
| `bancada/<placa>/com_framework/` | Logs UART, fotos, vídeos, capturas do analisador e observações da versão com framework. |
| `bancada/<placa>/sem_framework/` | Os mesmos tipos de registro para a versão convencional. |
| `firmwares/` | Identificação dos builds, configurações, logs de compilação e relatórios de memória. |
| `simulacao/` | Relatórios das suítes existentes, quando executadas para a versão final. |
| `metricas/` | Inventários de arquivos, contagens de código, dependências e dados de memória. |
| `../tcc_2/figuras/` | Figuras selecionadas para o artigo; preservar os registros originais em `bancada/`. |

As placas cadastradas são `esp32`, `nrf52840`, `stm32f407` e `stm32f411`. Cada uma possui as duas pastas de versão. Os arquivos `.gitkeep` permitem preservar as pastas inicialmente vazias no Git.

## Critério da campanha física

A campanha física avalia as oito versões com execuções documentadas e associadas aos binários finais. As aprovações atuais vêm dessas sessões; o funcionamento anterior à campanha e os testes simulados não as substituem. A coleta começou com todas as versões pendentes e está no estado resumido acima.

## Como preencher os resultados

- `PENDENTE`: campo ainda aguardando execução ou registro da campanha final.
- `APROVADO`: cenário executado e atendido, com evidência identificada.
- `FALHOU`: cenário executado com resultado diferente do esperado; descrever nas observações.
- `NAO_EXECUTADO`: ao fechar a campanha, cenário não realizado; registrar o motivo e a limitação.

No campo `evidencias`, informe os caminhos relativos a este diretório, por exemplo `bancada/esp32/com_framework/serial_20261104.log`. A identificação de caminhos não substitui a associação ao firmware de `controle.csv`.

Atualize a linha após cada execução. Se o firmware mudar, preserve os registros anteriores e identifique o novo binário e os ensaios repetidos. Não substitua registros brutos por imagens editadas para apresentação.

O campo `resultado` de `controle.csv` representa o ensaio físico; uma compilação aprovada, por si só, não o aprova. A data permanece vazia enquanto uma versão aguarda execução; nesta rodada, as oito já possuem data e resultado. Logs, relatórios e identificação das compilações estão em [firmwares/README.md](firmwares/README.md). Consulte o [roteiro de execução](../ROTEIRO.md) para completar as referências das capturas e consolidar o material do artigo.
