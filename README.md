# Evidências experimentais do framework para firmware

Material da avaliação de um framework modular em C em ESP32, nRF52840, STM32F407 e STM32F411. O experimento compara uma prova de conceito com e sem framework em cada placa: sensor SHT31-D, OLED SSD1306, botão, LED e UART.

Os builds foram identificados em **02/11/2026**; a campanha física ocorreu em **07–08/11/2026**. A publicação reúne os registros preservados, suas regras de interpretação e as revisões dos oito projetos. Esta publicação não representa uma nova execução dos ensaios.

## Conferir as evidências sem instalar os SDKs

```sh
git clone https://github.com/fmsantos8/testes_framework.git
cd testes_framework
python3 tools/conferir_publicacao.py
```

O verificador usa somente Python 3 e confere o inventário SHA-256, artefatos e configurações arquivados, diferenças de memória, oito versões e 64 campos funcionais, e o total de 290 leituras I2C completas. A integridade documental não substitui a interpretação das capturas nem constitui novo ensaio físico.

## Obter os fontes publicados

```sh
python3 tools/obter_fontes.py
python3 tools/conferir_publicacao.py --fontes .
```

O primeiro comando obtém os oito projetos nos commits fixados no [manifesto de publicação](manifesto-publicacao.json), com os submódulos correspondentes. Não compila nem grava placas. Também aceita `--projeto esp32_sem_framework` e `--destino /outro/diretorio`. Para compilar cada firmware, siga seu README e os requisitos dos SDKs registrados em [evidencias/firmwares/README.md](evidencias/firmwares/README.md).

| Plataforma | Com framework | Sem framework |
| --- | --- | --- |
| esp32 | [esp32_framework](https://github.com/fmsantos8/esp32_framework/tree/d1be40d4aed1d26b84899ffc01fa3563f6622450) | [esp32_sem_framework](https://github.com/fmsantos8/esp32_sem_framework/tree/aadda2b6c88c49c3c5a2c9881eed44b81a0695e7) |
| nrf52840 | [nrf52840_framework](https://github.com/fmsantos8/nrf52840_framework/tree/7dc83829f27ef718d47d8bc88932d14546baaa63) | [nrf52840_sem_framework](https://github.com/fmsantos8/nrf52840_sem_framework/tree/2b5f18c60d5a35708b953ae3cd9cbf9078e97bce) |
| stm32f407 | [stm32f407_framework](https://github.com/fmsantos8/stm32f407_framework/tree/45f2279c449b3a9d21a84cfcd940d36a5079c413) | [stm32f407_sem_framework](https://github.com/fmsantos8/stm32f407_sem_framework/tree/7e9dbaec81ad11c1e2643db8164e476dea00bbfc) |
| stm32f411 | [stm32f411_framework](https://github.com/fmsantos8/stm32f411_framework/tree/c0400e810d40fb6c73ed855bd1938392ee7b90c2) | [stm32f411_sem_framework](https://github.com/fmsantos8/stm32f411_sem_framework/tree/e7dec8b4808b5868ee5d2069c0b012060c69c718) |

## Contratos, portes e componentes

Os contratos públicos, a arena de memória, os três portes de SDK, os componentes e a aplicação comum são repositórios independentes, fixados por submódulos nos projetos com framework:

- [hal_interface](https://github.com/fmsantos8/hal_interface/tree/21b0e0fa80245341ca3daca9ad5f7df0ab289dd8): `21b0e0fa80245341ca3daca9ad5f7df0ab289dd8`.
- [esp32-wroom-32](https://github.com/fmsantos8/esp32-wroom-32/tree/0b7c43e15450fbeb8efacd3d6fbbab83e63cf1a8): `0b7c43e15450fbeb8efacd3d6fbbab83e63cf1a8`.
- [memory](https://github.com/fmsantos8/memory/tree/f62a76358003068210d619a29aae089ac4a302e5): `f62a76358003068210d619a29aae089ac4a302e5`.
- [ssd1306](https://github.com/fmsantos8/ssd1306/tree/99be092edeb78ea1d686e80792e613cda40fd864): `99be092edeb78ea1d686e80792e613cda40fd864`.
- [sht31d](https://github.com/fmsantos8/sht31d/tree/339e849ee0210856ab2f55c00d27ac51efd47d01): `339e849ee0210856ab2f55c00d27ac51efd47d01`.
- [button](https://github.com/fmsantos8/button/tree/e569415f6e04199cb5a2533f501b3e63a39ae56b): `e569415f6e04199cb5a2533f501b3e63a39ae56b`.
- [led](https://github.com/fmsantos8/led/tree/f589fc0a543d5d02a610597ec0349a82fd9d87f9): `f589fc0a543d5d02a610597ec0349a82fd9d87f9`.
- [telemetry](https://github.com/fmsantos8/telemetry/tree/4fc849774189ffa8a2b2c80a481d75c580119ece): `4fc849774189ffa8a2b2c80a481d75c580119ece`.
- [framework_app](https://github.com/fmsantos8/framework_app/tree/d3a0e1353f49643bb929a71e24329782b15e079a): `d3a0e1353f49643bb929a71e24329782b15e079a`.
- [nrf52840](https://github.com/fmsantos8/nrf52840/tree/3729cc2dc3f36f3ebb701c1377bba57430b75c0d): `3729cc2dc3f36f3ebb701c1377bba57430b75c0d`.
- [stm32f4](https://github.com/fmsantos8/stm32f4/tree/6ba6ffefbf2854645818d353fabfdbb998514760): `6ba6ffefbf2854645818d353fabfdbb998514760`.

## Como navegar pelos resultados

- [Estado da campanha](evidencias/README.md): identificação e resultados das oito versões.
- [Métricas](evidencias/metricas/RESUMO.md): código, origens compartilhadas, cópias idênticas, dependências e memória.
- [Tabelas e seleções editoriais](evidencias/artigo/README.md): tabelas, UART e limites usados na apresentação do artigo.
- [Campanha de bancada](evidencias/bancada/): sessões, fotos, bytes UART, capturas e observações.
- [Simulação preservada](evidencias/simulacao/README.md): conformidade dos portes e comparações de equivalência.
- [Prova de conceito convencional](docs/SEM_FRAMEWORK.md): organização e comportamento das implementações sem framework.

## Interpretação e proveniência

As revisões dos builds nos manifestos históricos foram preservadas. Os commits de publicação são posteriores e estão relacionados no manifesto de publicação por hashes dos arquivos; não se atribui retrospectivamente a execução dos builds a esses commits novos. Os fontes C/H e as configurações efetivamente avaliadas foram conferidos por conteúdo. A mudança de SSH para HTTPS em `.gitmodules` altera somente os endereços de obtenção: a versão histórica desse arquivo está em `configuracoes-historicas/`, e ambos os hashes estão documentados.

Os relatórios originais preservam caminhos absolutos do ambiente de execução e estados das etapas anteriores. As instruções desta página usam caminhos relativos ao novo repositório. Ferramentas históricas de compilação e coleta estão incluídas para consulta, mas sua execução exige os SDKs/toolchains descritos e pode produzir outros resultados; a conferência dos registros publicados é feita por `conferir_publicacao.py`.

O percentual de flash da ESP32 foi apresentado como **3,66%**, calculado diretamente dos bytes inteiros (7.344 / 200.928 × 100). O CSV histórico mantém seu percentual com quatro casas. As tabelas editoriais usam uma única etapa de arredondamento.

Os oito cenários foram publicados como aprovados nas oito versões. A conformidade física é limitada aos caminhos observados: o timer foi avaliado indiretamente pelo LED; interrupções de 1 ms não foram medidas diretamente. Simulação não substitui hardware. As intervenções adicionais e divergências de registro estão em [LIMITACOES.csv](evidencias/artigo/LIMITACOES.csv). Não foram medidos produtividade, esforço de manutenção ou pico de RAM.

## Executar novamente testes no computador

Depois de obter os fontes, com compiladores C/C++ disponíveis:

```sh
python3 validacao_sem_framework/run.py --output build/equivalencia
```

A suíte compara três cenários por placa, sem SDK nem placa física. Os resultados novos ficam em `build/` e não substituem os arquivados. A suíte de conformidade completa requer também o STM32CubeF4 fixado no manifesto:

```sh
python3 conformidade/run.py --stm32-sdk /caminho/STM32CubeF4 --build-manifest manifesto-equivalencia-publicacao.json --output build/conformidade
```

O manifesto de equivalência é uma cópia derivada que reconhece a atualização de URLs em `.gitmodules`; o manifesto histórico permanece intacto. Nenhum desses comandos grava microcontroladores.

As atribuições e licenças existentes nos fontes e dependências foram preservadas. Os SDKs devem ser obtidos das fontes oficiais indicadas pelos respectivos projetos.
