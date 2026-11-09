# Firmwares preparados — passo 2

Conclusão em 02/11/2026: **oito builds aprovados na compilação**, com os artefatos identificados e arquivados. Nenhuma placa foi gravada e nenhum ensaio físico foi executado. Os resultados funcionais permanecem pendentes.

## Onde estão os resultados

- [Resumo CSV](resumo.csv): uma linha por firmware, com ELF, hash, imagem para gravação futura, log e relatório de memória.
- [Manifesto](manifesto.json): comandos efetivamente executados, configurações, ferramentas, revisões dos SDKs, hashes dos fontes, artefatos e cópias das configurações efetivas.
- [Verificação final](verificacao.json): integridade dos arquivos, fontes correspondentes aos builds e comparação das opções de cada par.
- [Revisões dos submódulos](submodulos.json): commits carregados e fixados pelos projetos, limpeza dos submódulos e correspondência com os fontes compilados.
- [Ambiente local](ambiente.json): caminhos dos SDKs e ferramentas usados nesta rodada.
- [Controle das oito versões](../controle.csv): identificação preenchida; data e resultado dos ensaios físicos continuam pendentes.

Cada pasta de projeto contém:

```text
<projeto>/
    compilacao.log
    memoria.txt
    artefatos/
    configuracao/
```

`artefatos/` contém cópias dos ELF, imagens e MAP usados para identificar a versão. No ESP32, inclui também bootloader, tabela de partições e argumentos de gravação. No nRF, inclui HEX, BIN e UF2. `configuracao/` guarda os comandos de compilação por arquivo, cache CMake e configurações geradas pelo SDK.

Os caminhos e hashes de cada arquivo estão no manifesto. Os quatro projetos com framework possuem repositórios Git próprios, cujas revisões e alterações locais foram registradas no manifesto. Os oito submódulos comuns têm os mesmos commits nos quatro projetos; o porte STM32 tem o mesmo commit nas duas placas. A conferência dos commits carregados, dos gitlinks em HEAD/índice e da ausência de alterações locais está em `submodulos.json`. Os hashes dos fontes identificam também o conteúdo efetivamente compilado, incluindo os ajustes locais de integração.

## Configurações comparadas

| Par | Configuração efetiva e verificação |
| --- | --- |
| ESP32 | ESP-IDF v6.1, GCC Xtensa 15.2.0, `-Og`, assertions habilitadas e CPU de 160 MHz. Os `sdkconfig.json` gerados são idênticos, incluindo serviços, afinidade, stack e gerenciamento de energia. |
| nRF52840 | Subconjunto fixado do NCS v3.1.0, ARM GCC 14.3.1, Debug `-Og`, newlib, FPU e assertions habilitadas. Main stack de 4096 bytes, workqueue de 2048 e heap de 2048. |
| STM32F407 | STM32CubeF4 v1.28.3, ARM GCC 14.3.1, Debug `-Og -g3`, newlib-nano, FPU hard, assertions habilitadas, sem LTO, heap reservado de 512 bytes e stack de 4096. CPU configurada para 168 MHz nos dois projetos. |
| STM32F411 | Mesmas opções de ferramenta, biblioteca e reservas do par F407; CPU configurada para 100 MHz nos dois projetos. |

O nRF apresenta uma diferença no Kconfig gerado: `CONFIG_DT_HAS_GPIO_KEYS_ENABLED` existe apenas na versão convencional, em consequência do nó Devicetree usado para descrever seu botão. A comparação completa não encontrou outras diferenças de Kconfig; o marcador não habilita um serviço adicional neste build. O detalhe está na verificação JSON.

Os pinos, polaridades, I2C a 100 kHz, UART a 115200 8N1 e a base nominal de 1 ms foram conferidos nas configurações dos pares. Isso é conferência de configuração em código; as grandezas físicas e o comportamento serão observados em bancada.

Foram confirmados os mesmos 45 arquivos C/H comuns nos quatro projetos com framework: 15 de aplicação, 21 de componentes, sete de interfaces e dois da arena. Os fontes C/H do porte STM32 também são idênticos nos dois projetos.

### Ajuste necessário para uma comparação justa

As versões STM32 com framework usavam `-O0` em Debug e a biblioteca C completa; as convencionais usavam `-Og` e newlib-nano. Foram ajustados apenas os arquivos `cmake/flags.cmake` dos dois projetos com framework para usar `-Og -g3` e `--specs=nano.specs`. As opções efetivas foram verificadas nos comandos de compilação e link. A configuração comparada nesta rodada é Debug; os builds antigos não fundamentam os novos números.

Os relatórios de memória foram preservados como dados brutos. O cálculo comparativo e sua interpretação pertencem ao passo 6. Tamanho do ELF em disco não representa consumo de flash; os dados estáticos também não demonstram o pico de RAM em execução.

## Ferramentas e dependências

Os compiladores instalados no computador foram reutilizados. Os SDKs antes localizados em diretórios temporários ausentes foram restaurados em `build/deps/`, fora dos projetos de firmware e ignorados pelo Git:

- STM32CubeF4: raiz `94cae6e83f00e276a11957e7833c01ac3d0bd7af`, CMSIS Device `3c77349ce04c8af401454cc51f85ea9a50e34fc1` e HAL `b6f0ed3829f3829eb358a2e7417d80bba1a42db7`.
- NCS: revisões fixadas em `nrf52840_framework/tools/sdk-lock.json`, iguais às da versão convencional; as revisões instaladas estão no manifesto.
- Ambiente Python local em `build/deps/venv/`, com as dependências declaradas pelos projetos nRF. As listas efetivas de pacotes estão em [dependências nRF](dependencias-python-nrf.json) e [dependências ESP32](dependencias-python-esp32.json).

Os logs preservam avisos dos SDKs: parâmetros não utilizados no driver ST, ausência do DTC externo no Zephyr e avisos de segmento RWX do linker, além de mensagens de configuração do ESP-IDF. Os builds concluíram; isso não converte esses resultados em aprovação de hardware. No nRF, a geração Devicetree foi executada pelas ferramentas Python do SDK; o DTC externo não foi usado nesta rodada.

## Como conferir ou reproduzir

Para conferir os arquivos atuais sem compilar nem acessar placas, na raiz:

```bash
python3 tools/verificar_firmwares.py
```

Para recompilar e arquivar, revise os caminhos de `ambiente.json` se mudar de computador. As dependências devem estar instaladas conforme os READMEs e arquivos de versões dos projetos. Execute na raiz:

```bash
python3 tools/preparar_firmwares.py
```

Também é possível selecionar um projeto com `--project stm32f411_framework`, por exemplo. A preparação usa os mesmos SDKs e ferramentas nos pares, produz logs e nunca acessa placas. Uma nova execução substitui o registro e os arquivos selecionados: preserve esta pasta e os resultados associados antes de reconstruir outra revisão. O controle CSV deve ser atualizado se os binários mudarem.

## Gravação adiada

A gravação será feita posteriormente, **uma placa e uma versão por vez**, após o fechamento das tarefas de computador. Antes de gravar, conferir o hash do arquivo selecionado com o manifesto. Os scripts não devem compilar automaticamente outra revisão sem atualizar a identificação.

Os arquivos `flash_args`/`flasher_args.json` do ESP32 foram preservados como produzidos pelo SDK e referenciam o diretório de build original. Para gravar a partir da cópia arquivada, é necessário resolver os caminhos para essas cópias ou verificar os arquivos originais antes de usá-los. No nRF, a imagem UF2 desta configuração pressupõe o bootloader compatível e início de aplicação em `0x26000`.
