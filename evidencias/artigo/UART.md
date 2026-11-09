# Seleção da UART do STM32F411 com framework

Fonte: [serial_20261108_180238_513639.log](../bancada/stm32f411/com_framework/sessao_20261108_180202_079953/serial_20261108_180238_513639.log).

Horários são de recepção no computador. Cada bloco conserva o texto literal das linhas indicadas; as lacunas entre blocos são cortes editoriais. A ausência de novo ERROR nos blocos finais complementa a recuperação visual confirmada nas observações; sozinha não comprova a tela. O arquivo .bin preserva os bytes CR LF no fio.

## Leitura normal e falha do sensor

Linhas 29–31 do log original.

```text
[18:03:36.808502] SAMPLE;temperature_mC=23638;humidity_mpercent=48920
[18:03:38.813272] ERROR;source=SHT31D;code=6
[18:03:40.816146] ERROR;source=SHT31D;code=6
```

## Recuperação do sensor

Linhas 47–49 do log original.

```text
[18:04:13.856760] ERROR;source=SHT31D;code=6
[18:04:15.864641] SAMPLE;temperature_mC=26821;humidity_mpercent=57127
[18:04:18.068223] SAMPLE;temperature_mC=26778;humidity_mpercent=52395
```

## Falha do display com aquisição mantida

Linhas 55–58 do log original.

```text
[18:04:27.278991] ERROR;source=SSD1306;code=5
[18:04:28.280193] SAMPLE;temperature_mC=25887;humidity_mpercent=45460
[18:04:28.280223] ERROR;source=SSD1306;code=5
[18:04:29.281444] ERROR;source=SSD1306;code=5
```

## Recuperação do display

Linhas 81–83 do log original.

```text
[18:04:44.701909] ERROR;source=SSD1306;code=5
[18:04:46.704276] SAMPLE;temperature_mC=24981;humidity_mpercent=48681
[18:04:48.906992] SAMPLE;temperature_mC=24866;humidity_mpercent=47437
```
