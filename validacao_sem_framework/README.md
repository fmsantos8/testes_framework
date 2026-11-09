# Equivalência das implementações convencionais

Execute na raiz do repositório, com Python 3 e um compilador C nativo disponível como `cc`:

```sh
python3 validacao_sem_framework/run.py
```

A suíte compila os **fontes reais** das quatro implementações convencionais e o núcleo real da implementação com framework. Antes, verifica a identidade dos arquivos C/H de aplicação e componentes nas quatro referências; a referência ESP32 só é utilizada como origem comum depois dessa verificação.

A fronteira de hardware é substituída por stubs: a HAL portável no programa de referência e as chamadas dos SDKs nos programas convencionais. Cada executável recebe a mesma sequência de entradas e falhas. As traces comparam exatamente cada byte de comandos e dados I2C, cada prefixo aceito pela UART, códigos de erro, instantes nominais e transições do LED. Não há filtragem de diferenças: a inicialização do GPIO nativo é representada no adaptador de teste.

São executados três cenários por plataforma:

1. Contador começando em zero: inicialização, aquisição de 2 s, conversão de 16 notificações, extremos do sensor, CRC inválido, timeout e erro I2C, retenção da amostra, debounce com bounce e pressionamento mantido, duas telas, páginas pendentes e recuperação do display, LED, UART sem prefixo/parcial, repetição de delimitador e timeout após aceitar todos os bytes.
2. Mesmo roteiro começando perto de `UINT32_MAX`: retorno do contador a zero, uptime acumulado e diagnóstico em dias.
3. Indisponibilidade UART prolongada combinada com falhas do display: saturação da fila de oito registros, descarte de novos eventos quando cheia e recuperação após a comunicação voltar.

O compilador usa `-Wall -Wextra -Werror` e UndefinedBehaviorSanitizer, sem recuperação após erros. Traces, diferenças de falhas, hashes dos fontes e `results.json` ficam em `build/validacao_sem_framework/`. Use `--output` para escolher outro destino. Uma divergência ou falha de compilação termina com código diferente de zero.

**Limite:** isto demonstra equivalência do software sob as entradas simuladas, sem comprovar funcionamento elétrico, temporização física, interrupções reais, clock, abort ou EasyDMA. O adaptador nRF substitui os helpers de transação nrfx; `peripherals.c` é verificado pela compilação com o SDK real, não pela simulação host. A evidência física exige os ensaios nas placas.

Os stubs permanecem exclusivamente neste diretório. Nenhum cabeçalho simulado participa dos builds de produção.
